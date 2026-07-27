/* Generated from sail/executor/receipts.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void create_letbind_309(void) {

  uint64_t z3zE335;
  z3zE335 = UINT64_C(256);
  zLOGS_BLOOM_BYTE_LENGTH = z3zE335;
let_end_969: ;
}
void kill_letbind_309(void) {
}

uint64_t zbloom_bit_mask(uint64_t zbit_to_set)
{
  uint64_t z8zE632;
  z8zE632 = ((zbit_to_set >= UINT64_C(64)) ? UINT64_C(0) : ((UINT64_C(0x01) << zbit_to_set) & UINT64_C(0xFF)));
end_function_1361: ;
  return z8zE632;
end_block_exception_1362: ;

  return UINT64_C(0xdeadc0de);
}

sail_fixed_bytes_256 zbloom_set_bit(sail_fixed_bytes_256 zbloom, uint64_t zbit_to_set)
{
  sail_fixed_bytes_256 z8zE633;
  sail_fixed_bytes_256 zout;
  zout = zbloom;
  uint64_t zquotient;
  zquotient = (zbit_to_set / UINT64_C(8));
  uint64_t znatural_byte;
  {
    bool z2zE467;
    z2zE467 = (!(UINT64_C(255) < zquotient));
    znatural_byte = zquotient;
  }
  uint64_t zremainder;
  {    zremainder = (zbit_to_set % UINT64_C(8));
  }
  uint64_t zbit_in_byte;
  {
    bool z2zE466;
    z2zE466 = (!(UINT64_C(7) < zremainder));
    zbit_in_byte = zremainder;
  }
  uint64_t z2zE465;
  {
    uint64_t z2zE463;
    z2zE463 = fast_unsigned_vector_access_fixed_bytes_256(zout, znatural_byte);
    uint64_t z2zE464;
    z2zE464 = zbloom_bit_mask(zbit_in_byte);
    z2zE465 = (z2zE463 | z2zE464);
  }
  zout = fast_unsigned_vector_update_fixed_bytes_256(zout, znatural_byte, z2zE465);
  unit z3zE784;
  z3zE784 = UNIT;
  z8zE633 = zout;
end_function_1359: ;
  return z8zE633;
end_block_exception_1360: ;

  return fixed_bytes_256_zero();
}

sail_fixed_bytes_256 zbloom_add_entry_hash(sail_fixed_bytes_256 zbloom, sail_fixed_bytes_32 zh)
{
  sail_fixed_bytes_256 z8zE634;
  sail_fixed_bytes_32 zbytes;
  zbytes = zh;
  sail_fixed_bytes_256 zout;
  {
    uint64_t z2zE462;
    {
      uint64_t z2zE461;
      {
        uint64_t z2zE459;
        {
          uint64_t z2zE458;
          z2zE458 = fast_unsigned_vector_access_fixed_bytes_32(zbytes, UINT64_C(0));
          z2zE459 = (safe_rshift(UINT64_MAX, 64 - 3) & (z2zE458 >> UINT64_C(0)));
        }
        uint64_t z2zE460;
        z2zE460 = fast_unsigned_vector_access_fixed_bytes_32(zbytes, UINT64_C(1));
        z2zE461 = (z2zE459 << 8) | z2zE460;
      }
      z2zE462 = ((uint64_t) z2zE461);
    }
    zout = zbloom_set_bit(zbloom, z2zE462);
  }
  uint64_t z2zE452;
  {
    uint64_t z2zE451;
    {
      uint64_t z2zE449;
      {
        uint64_t z2zE448;
        z2zE448 = fast_unsigned_vector_access_fixed_bytes_32(zbytes, UINT64_C(2));
        z2zE449 = (safe_rshift(UINT64_MAX, 64 - 3) & (z2zE448 >> UINT64_C(0)));
      }
      uint64_t z2zE450;
      z2zE450 = fast_unsigned_vector_access_fixed_bytes_32(zbytes, UINT64_C(3));
      z2zE451 = (z2zE449 << 8) | z2zE450;
    }
    z2zE452 = ((uint64_t) z2zE451);
  }
  zout = zbloom_set_bit(zout, z2zE452);
  unit z3zE783;
  z3zE783 = UNIT;
  uint64_t z2zE457;
  {
    uint64_t z2zE456;
    {
      uint64_t z2zE454;
      {
        uint64_t z2zE453;
        z2zE453 = fast_unsigned_vector_access_fixed_bytes_32(zbytes, UINT64_C(4));
        z2zE454 = (safe_rshift(UINT64_MAX, 64 - 3) & (z2zE453 >> UINT64_C(0)));
      }
      uint64_t z2zE455;
      z2zE455 = fast_unsigned_vector_access_fixed_bytes_32(zbytes, UINT64_C(5));
      z2zE456 = (z2zE454 << 8) | z2zE455;
    }
    z2zE457 = ((uint64_t) z2zE456);
  }
  zout = zbloom_set_bit(zout, z2zE457);
  unit z3zE782;
  z3zE782 = UNIT;
  z8zE634 = zout;
end_function_1357: ;
  return z8zE634;
end_block_exception_1358: ;

  return fixed_bytes_256_zero();
}

sail_fixed_bytes_256 zbloom_add_topics(sail_fixed_bytes_256 zbloom, zz5listz8z5structz0zz__sail_c_repr_u256z9 ztopics)
{
  sail_fixed_bytes_256 z8zE635;
  sail_fixed_bytes_256 z3zE779;
  {
    if (!((ztopics == NULL))) goto case_1354;
    z3zE779 = zbloom;
    goto finish_match_1352;
  }
case_1354: ;
  {
    /* complete */
    sail_u256 ztopic;
    ztopic = (*ztopics).hd;
    zz5listz8z5structz0zz__sail_c_repr_u256z9 zrest;
    CREATE(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&zrest);
    COPY(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&zrest, (*ztopics).tl);
    sail_fixed_bytes_256 z2zE447;
    {
      sail_fixed_bytes_32 z2zE446;
      z2zE446 = zkeccak256_word(ztopic);
      z2zE447 = zbloom_add_entry_hash(zbloom, z2zE446);
    }
    z3zE779 = zbloom_add_topics(z2zE447, zrest);
    KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&zrest);
    goto finish_match_1352;
  }
case_1353: ;
finish_match_1352: ;
  z8zE635 = z3zE779;
end_function_1355: ;
  return z8zE635;
end_block_exception_1356: ;

  return fixed_bytes_256_zero();
}

sail_fixed_bytes_256 zbloom_add_log(sail_fixed_bytes_256 zbloom, struct zLogEntry zlog)
{
  sail_fixed_bytes_256 z8zE636;
  sail_fixed_bytes_256 zwith_address;
  {
    sail_fixed_bytes_32 z2zE445;
    {
      sail_fixed_bytes_20 z2zE444;
      z2zE444 = zlog.zaddress;
      z2zE445 = zkeccak256_address(z2zE444);
    }
    zwith_address = zbloom_add_entry_hash(zbloom, z2zE445);
  }
  zz5listz8z5structz0zz__sail_c_repr_u256z9 z2zE443;
  CREATE(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z2zE443);
  COPY(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z2zE443, zlog.ztopics);
  z8zE636 = zbloom_add_topics(zwith_address, z2zE443);
  KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z2zE443);
end_function_1350: ;
  return z8zE636;
end_block_exception_1351: ;

  return fixed_bytes_256_zero();
}

sail_fixed_bytes_256 zbloom_add_logs(sail_fixed_bytes_256 zbloom, zz5listz8z5structz0zzLogEntryz9 zlogs)
{
  sail_fixed_bytes_256 z8zE637;
  sail_fixed_bytes_256 z3zE776;
  {
    if (!((zlogs == NULL))) goto case_1347;
    z3zE776 = zbloom;
    goto finish_match_1345;
  }
case_1347: ;
  {
    /* complete */
    struct zLogEntry zlog;
    CREATE(zLogEntry)(&zlog);
    COPY(zLogEntry)(&zlog, (*zlogs).hd);
    zz5listz8z5structz0zzLogEntryz9 zrest;
    CREATE(zz5listz8z5structz0zzLogEntryz9)(&zrest);
    COPY(zz5listz8z5structz0zzLogEntryz9)(&zrest, (*zlogs).tl);
    sail_fixed_bytes_256 z2zE442;
    z2zE442 = zbloom_add_log(zbloom, zlog);
    z3zE776 = zbloom_add_logs(z2zE442, zrest);
    KILL(zz5listz8z5structz0zzLogEntryz9)(&zrest);
    KILL(zLogEntry)(&zlog);
    goto finish_match_1345;
  }
case_1346: ;
finish_match_1345: ;
  z8zE637 = z3zE776;
end_function_1348: ;
  return z8zE637;
end_block_exception_1349: ;

  return fixed_bytes_256_zero();
}

sail_fixed_bytes_256 zlogs_bloom_for_logs(zz5listz8z5structz0zzLogEntryz9 zlogs)
{
  sail_fixed_bytes_256 z8zE638;
  z8zE638 = zbloom_add_logs(zEMPTY_LOGS_BLOOM, zlogs);
end_function_1343: ;
  return z8zE638;
end_block_exception_1344: ;

  return fixed_bytes_256_zero();
}

uint64_t ztopics_rlp_content_sizze(zz5listz8z5structz0zz__sail_c_repr_u256z9 ztopics)
{
  uint64_t z8zE639;
  uint64_t z3zE773;
  {
    if (!((ztopics == NULL))) goto case_1340;
    z3zE773 = UINT64_C(0);
    goto finish_match_1338;
  }
case_1340: ;
  {
    /* complete */
    zz5listz8z5structz0zz__sail_c_repr_u256z9 zrest;
    CREATE(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&zrest);
    COPY(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&zrest, (*ztopics).tl);
    uint64_t zrest_length;
    {
      zrest_length = ztopics_rlp_content_sizze(zrest);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&zrest);
        goto end_block_exception_1342;
      }
    }
    {
      z3zE773 = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(UINT64_C(33), zrest_length);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&zrest);
        goto end_block_exception_1342;
      }
    }
    KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&zrest);
    goto finish_match_1338;
  }
case_1339: ;
finish_match_1338: ;
  z8zE639 = z3zE773;
end_function_1341: ;
  return z8zE639;
end_block_exception_1342: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t ztopics_rlp_sizze(zz5listz8z5structz0zz__sail_c_repr_u256z9 ztopics)
{
  uint64_t z8zE640;
  uint64_t z2zE441;
  {
    z2zE441 = ztopics_rlp_content_sizze(ztopics);
    if (have_exception) {  goto end_block_exception_1337;  }
  }
  {
    z8zE640 = zrlp_scratch_list_sizze(z2zE441);
    if (have_exception) {  goto end_block_exception_1337;  }
  }
end_function_1336: ;
  return z8zE640;
end_block_exception_1337: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zlog_entry_rlp_content_sizze(struct zLogEntry zlog)
{
  uint64_t z8zE641;
  uint64_t ztopics_length;
  {
    zz5listz8z5structz0zz__sail_c_repr_u256z9 z2zE440;
    CREATE(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z2zE440);
    COPY(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z2zE440, zlog.ztopics);
    {
      ztopics_length = ztopics_rlp_sizze(z2zE440);
      if (have_exception) {
        KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z2zE440);
        goto end_block_exception_1335;
      }
    }
    KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z2zE440);
  }
  uint64_t zdata_length;
  {
    struct zByteSliceFields z2zE439;
    z2zE439 = zlog.zdata;
    {
      zdata_length = zrlp_scratch_slice_sizze(z2zE439);
      if (have_exception) {  goto end_block_exception_1335;  }
    }
  }
  uint64_t z2zE438;
  {
    z2zE438 = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(UINT64_C(21), ztopics_length);
    if (have_exception) {  goto end_block_exception_1335;  }
  }
  {
    z8zE641 = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE438, zdata_length);
    if (have_exception) {  goto end_block_exception_1335;  }
  }
end_function_1334: ;
  return z8zE641;
end_block_exception_1335: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zlog_entry_rlp_sizze(struct zLogEntry zlog)
{
  uint64_t z8zE642;
  uint64_t z2zE437;
  {
    z2zE437 = zlog_entry_rlp_content_sizze(zlog);
    if (have_exception) {  goto end_block_exception_1333;  }
  }
  {
    z8zE642 = zrlp_scratch_list_sizze(z2zE437);
    if (have_exception) {  goto end_block_exception_1333;  }
  }
end_function_1332: ;
  return z8zE642;
end_block_exception_1333: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zlogs_rlp_content_sizze(zz5listz8z5structz0zzLogEntryz9 zlogs)
{
  uint64_t z8zE643;
  uint64_t z3zE770;
  {
    if (!((zlogs == NULL))) goto case_1329;
    z3zE770 = UINT64_C(0);
    goto finish_match_1327;
  }
case_1329: ;
  {
    /* complete */
    struct zLogEntry zlog;
    CREATE(zLogEntry)(&zlog);
    COPY(zLogEntry)(&zlog, (*zlogs).hd);
    zz5listz8z5structz0zzLogEntryz9 zrest;
    CREATE(zz5listz8z5structz0zzLogEntryz9)(&zrest);
    COPY(zz5listz8z5structz0zzLogEntryz9)(&zrest, (*zlogs).tl);
    uint64_t zlog_length;
    {
      zlog_length = zlog_entry_rlp_sizze(zlog);
      if (have_exception) {
        KILL(zLogEntry)(&zlog);
        KILL(zz5listz8z5structz0zzLogEntryz9)(&zrest);
        goto end_block_exception_1331;
      }
    }
    uint64_t zrest_length;
    {
      zrest_length = zlogs_rlp_content_sizze(zrest);
      if (have_exception) {
        KILL(zLogEntry)(&zlog);
        KILL(zz5listz8z5structz0zzLogEntryz9)(&zrest);
        goto end_block_exception_1331;
      }
    }
    {
      z3zE770 = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zlog_length, zrest_length);
      if (have_exception) {
        KILL(zLogEntry)(&zlog);
        KILL(zz5listz8z5structz0zzLogEntryz9)(&zrest);
        goto end_block_exception_1331;
      }
    }
    KILL(zz5listz8z5structz0zzLogEntryz9)(&zrest);
    KILL(zLogEntry)(&zlog);
    goto finish_match_1327;
  }
case_1328: ;
finish_match_1327: ;
  z8zE643 = z3zE770;
end_function_1330: ;
  return z8zE643;
end_block_exception_1331: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zlogs_rlp_sizze(zz5listz8z5structz0zzLogEntryz9 zlogs)
{
  uint64_t z8zE644;
  uint64_t z2zE436;
  {
    z2zE436 = zlogs_rlp_content_sizze(zlogs);
    if (have_exception) {  goto end_block_exception_1326;  }
  }
  {
    z8zE644 = zrlp_scratch_list_sizze(z2zE436);
    if (have_exception) {  goto end_block_exception_1326;  }
  }
end_function_1325: ;
  return z8zE644;
end_block_exception_1326: ;

  return UINT64_C(0xdeadc0de);
}

unit zrlp_write_topics_content(zz5listz8z5structz0zz__sail_c_repr_u256z9 ztopics)
{
  unit z8zE645;
  unit z3zE766;
  {
    if (!((ztopics == NULL))) goto case_1322;
    z3zE766 = UNIT;
    goto finish_match_1320;
  }
case_1322: ;
  {
    /* complete */
    sail_u256 ztopic;
    ztopic = (*ztopics).hd;
    zz5listz8z5structz0zz__sail_c_repr_u256z9 zrest;
    CREATE(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&zrest);
    COPY(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&zrest, (*ztopics).tl);
    unit z3zE767;
    z3zE767 = zrlp_write_word(ztopic);
    z3zE766 = zrlp_write_topics_content(zrest);
    KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&zrest);
    goto finish_match_1320;
  }
case_1321: ;
finish_match_1320: ;
  z8zE645 = z3zE766;
end_function_1323: ;
  return z8zE645;
end_block_exception_1324: ;

  return UNIT;
}

unit zrlp_write_topics(zz5listz8z5structz0zz__sail_c_repr_u256z9 ztopics)
{
  unit z8zE646;
  uint64_t z2zE435;
  {
    z2zE435 = ztopics_rlp_content_sizze(ztopics);
    if (have_exception) {  goto end_block_exception_1319;  }
  }
  unit z3zE765;
  z3zE765 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE435);
  z8zE646 = zrlp_write_topics_content(ztopics);
end_function_1318: ;
  return z8zE646;
end_block_exception_1319: ;

  return UNIT;
}

unit zrlp_write_log_entry(struct zLogEntry zlog)
{
  unit z8zE647;
  uint64_t z2zE431;
  {
    z2zE431 = zlog_entry_rlp_content_sizze(zlog);
    if (have_exception) {  goto end_block_exception_1317;  }
  }
  unit z3zE764;
  z3zE764 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE431);
  sail_fixed_bytes_20 z2zE432;
  z2zE432 = zlog.zaddress;
  unit z3zE763;
  z3zE763 = zrlp_write_addr(z2zE432);
  zz5listz8z5structz0zz__sail_c_repr_u256z9 z2zE433;
  CREATE(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z2zE433);
  COPY(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z2zE433, zlog.ztopics);
  unit z3zE762;
  {
    z3zE762 = zrlp_write_topics(z2zE433);
    if (have_exception) {
      KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z2zE433);
      goto end_block_exception_1317;
    }
  }
  KILL(zz5listz8z5structz0zz__sail_c_repr_u256z9)(&z2zE433);
  struct zByteSliceFields z2zE434;
  z2zE434 = zlog.zdata;
  z8zE647 = zrlp_write_slice(z2zE434);
end_function_1316: ;
  return z8zE647;
end_block_exception_1317: ;

  return UNIT;
}

unit zrlp_write_logs_content(zz5listz8z5structz0zzLogEntryz9 zlogs)
{
  unit z8zE648;
  unit z3zE758;
  {
    if (!((zlogs == NULL))) goto case_1313;
    z3zE758 = UNIT;
    goto finish_match_1311;
  }
case_1313: ;
  {
    /* complete */
    struct zLogEntry zlog;
    CREATE(zLogEntry)(&zlog);
    COPY(zLogEntry)(&zlog, (*zlogs).hd);
    zz5listz8z5structz0zzLogEntryz9 zrest;
    CREATE(zz5listz8z5structz0zzLogEntryz9)(&zrest);
    COPY(zz5listz8z5structz0zzLogEntryz9)(&zrest, (*zlogs).tl);
    unit z3zE759;
    {
      z3zE759 = zrlp_write_log_entry(zlog);
      if (have_exception) {
        KILL(zLogEntry)(&zlog);
        KILL(zz5listz8z5structz0zzLogEntryz9)(&zrest);
        goto end_block_exception_1315;
      }
    }
    {
      z3zE758 = zrlp_write_logs_content(zrest);
      if (have_exception) {
        KILL(zLogEntry)(&zlog);
        KILL(zz5listz8z5structz0zzLogEntryz9)(&zrest);
        goto end_block_exception_1315;
      }
    }
    KILL(zz5listz8z5structz0zzLogEntryz9)(&zrest);
    KILL(zLogEntry)(&zlog);
    goto finish_match_1311;
  }
case_1312: ;
finish_match_1311: ;
  z8zE648 = z3zE758;
end_function_1314: ;
  return z8zE648;
end_block_exception_1315: ;

  return UNIT;
}

unit zrlp_write_logs(zz5listz8z5structz0zzLogEntryz9 zlogs)
{
  unit z8zE649;
  uint64_t z2zE430;
  {
    z2zE430 = zlogs_rlp_content_sizze(zlogs);
    if (have_exception) {  goto end_block_exception_1310;  }
  }
  unit z3zE757;
  z3zE757 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE430);
  {
    z8zE649 = zrlp_write_logs_content(zlogs);
    if (have_exception) {  goto end_block_exception_1310;  }
  }
end_function_1309: ;
  return z8zE649;
end_block_exception_1310: ;

  return UNIT;
}

uint64_t zreceipt_payload_content_sizze(struct zReceipt zr, uint64_t zcumulative_gas_used)
{
  uint64_t z8zE650;
  uint64_t zstatus;
  {
    bool z2zE429;
    z2zE429 = zr.zsuccess;
    if (z2zE429) {  zstatus = UINT64_C(1);  } else {  zstatus = UINT64_C(0);  }
  }
  uint64_t zstatus_length;
  {
    uint64_t z2zE428;
    z2zE428 = zrlp_uint_word_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zstatus);
    zstatus_length = zrlp_scratch_small_length(z2zE428);
  }
  sail_u256 zgas_word;
  zgas_word = zword_of_nat_byte_countzIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zcumulative_gas_used);
  uint64_t zgas_length;
  {
    uint64_t z2zE427;
    z2zE427 = zrlp_uint_word_sizze(zgas_word);
    zgas_length = zrlp_scratch_small_length(z2zE427);
  }
  uint64_t zbloom_length;
  {
    uint64_t z2zE426;
    {
      uint64_t z2zE425;
      z2zE425 = zrlp_length_prefix_lenzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zLOGS_BLOOM_BYTE_LENGTH);
      z2zE426 = zrlp_scratch_small_length(z2zE425);
    }
    {
      zbloom_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zLOGS_BLOOM_BYTE_LENGTH, z2zE426);
      if (have_exception) {  goto end_block_exception_1308;  }
    }
  }
  uint64_t zlogs_length;
  {
    zz5listz8z5structz0zzLogEntryz9 z2zE424;
    CREATE(zz5listz8z5structz0zzLogEntryz9)(&z2zE424);
    COPY(zz5listz8z5structz0zzLogEntryz9)(&z2zE424, zr.zlogs);
    {
      zlogs_length = zlogs_rlp_sizze(z2zE424);
      if (have_exception) {
        KILL(zz5listz8z5structz0zzLogEntryz9)(&z2zE424);
        goto end_block_exception_1308;
      }
    }
    KILL(zz5listz8z5structz0zzLogEntryz9)(&z2zE424);
  }
  uint64_t zfixed_length;
  {
    zfixed_length = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zstatus_length, zgas_length);
    if (have_exception) {  goto end_block_exception_1308;  }
  }
  uint64_t z2zE423;
  {
    z2zE423 = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zfixed_length, zbloom_length);
    if (have_exception) {  goto end_block_exception_1308;  }
  }
  {
    z8zE650 = zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE423, zlogs_length);
    if (have_exception) {  goto end_block_exception_1308;  }
  }
end_function_1307: ;
  return z8zE650;
end_block_exception_1308: ;

  return UINT64_C(0xdeadc0de);
}

struct zByteSliceFields zreceipt_encoded(struct zReceipt zr, uint64_t zcumulative_gas_used)
{
  struct zByteSliceFields z8zE651;
  uint64_t zstatus;
  {
    bool z2zE422;
    z2zE422 = zr.zsuccess;
    if (z2zE422) {  zstatus = UINT64_C(1);  } else {  zstatus = UINT64_C(0);  }
  }
  zz5listz8z5bv8z9 zbloom;
  CREATE(zz5listz8z5bv8z9)(&zbloom);
  {
    sail_fixed_bytes_256 z2zE421;
    {
      zz5listz8z5structz0zzLogEntryz9 z2zE420;
      CREATE(zz5listz8z5structz0zzLogEntryz9)(&z2zE420);
      COPY(zz5listz8z5structz0zzLogEntryz9)(&z2zE420, zr.zlogs);
      z2zE421 = zlogs_bloom_for_logs(z2zE420);
      KILL(zz5listz8z5structz0zzLogEntryz9)(&z2zE420);
    }
    zlogs_bloom_bytes(&zbloom, z2zE421);
  }
  uint64_t zcontent_len;
  {
    zcontent_len = zreceipt_payload_content_sizze(zr, zcumulative_gas_used);
    if (have_exception) {
      KILL(zz5listz8z5bv8z9)(&zbloom);
      goto end_block_exception_1306;
    }
  }
  bool ztyped;
  {
    uint64_t z2zE419;
    {
      enum zTxType z2zE418;
      z2zE418 = zr.ztx_type;
      z2zE419 = ztx_type_byte(z2zE418);
    }
    ztyped = (z2zE419 != UINT64_C(0x00));
  }
  uint64_t zstart;
  zstart = zscratch_begin(UNIT);
  unit z3zE755;
  if (ztyped) {
    zz5listz8z5bv8z9 z2zE416;
    CREATE(zz5listz8z5bv8z9)(&z2zE416);
    {
      uint64_t z2zE415;
      {
        enum zTxType z2zE414;
        z2zE414 = zr.ztx_type;
        z2zE415 = ztx_type_byte(z2zE414);
      }
      zconsz3z5bv8(&z2zE416, z2zE415, z2zE416);
    }
    z3zE755 = zscratch_push_bytes(z2zE416, UINT64_C(1));
    KILL(zz5listz8z5bv8z9)(&z2zE416);
  } else {  z3zE755 = UNIT;  }
  unit z3zE754;
  z3zE754 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zcontent_len);
  unit z3zE753;
  z3zE753 = zrlp_write_uint_wordzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zstatus);
  unit z3zE752;
  z3zE752 = zrlp_write_uint_nat(zcumulative_gas_used);
  unit z3zE751;
  z3zE751 = zrlp_write_bytes(zbloom, zLOGS_BLOOM_BYTE_LENGTH);
  zz5listz8z5structz0zzLogEntryz9 z2zE417;
  CREATE(zz5listz8z5structz0zzLogEntryz9)(&z2zE417);
  COPY(zz5listz8z5structz0zzLogEntryz9)(&z2zE417, zr.zlogs);
  unit z3zE750;
  {
    z3zE750 = zrlp_write_logs(z2zE417);
    if (have_exception) {
      KILL(zz5listz8z5bv8z9)(&zbloom);
      KILL(zz5listz8z5structz0zzLogEntryz9)(&z2zE417);
      goto end_block_exception_1306;
    }
  }
  KILL(zz5listz8z5structz0zzLogEntryz9)(&z2zE417);
  z8zE651 = zrlp_finish(zstart);
  KILL(zz5listz8z5bv8z9)(&zbloom);
end_function_1305: ;
  return z8zE651;
end_block_exception_1306: ;
  struct zByteSliceFields z8zE994 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE994;
}

struct zReceiptAccumulator zreceipt_accumulator_empty(unit z3zE747)
{
  struct zReceiptAccumulator z8zE652;
  unit z3zE748;
  z3zE748 = evmsail_receipt_table_reset(UNIT);
  struct zReceiptAccumulator z3zE749;
  z3zE749.zbloom = zEMPTY_LOGS_BLOOM;
  z3zE749.zcount = UINT64_C(0);
  z3zE749.zcumulative_gas_used = UINT64_C(0);
  z8zE652 = z3zE749;
end_function_1303: ;
  return z8zE652;
end_block_exception_1304: ;
  struct zReceiptAccumulator z8zE995 = { .zbloom = fixed_bytes_256_zero(), .zcount = UINT64_C(0xdeadc0de), .zcumulative_gas_used = UINT64_C(0xdeadc0de) };
  return z8zE995;
}

struct zReceiptAccumulator zreceipt_accumulator_push(struct zReceiptAccumulator zacc, struct zReceipt zreceipt, uint64_t znext_count)
{
  struct zReceiptAccumulator z8zE653;
  uint64_t zcumulative;
  {
    uint64_t z2zE412;
    z2zE412 = zacc.zcumulative_gas_used;
    uint64_t z2zE413;
    z2zE413 = zreceipt.zgas_used;
    {
      zcumulative = zconserved_gas_add(z2zE412, z2zE413);
      if (have_exception) {  goto end_block_exception_1302;  }
    }
  }
  uint64_t zmark;
  zmark = zscratch_begin(UNIT);
  struct zByteSliceFields zvalue;
  {
    zvalue = zreceipt_encoded(zreceipt, zcumulative);
    if (have_exception) {  goto end_block_exception_1302;  }
  }
  struct zOptimizzedUnitResult z2zE408;
  CREATE(zOptimizzedUnitResult)(&z2zE408);
  {
    uint64_t z2zE407;
    z2zE407 = zacc.zcount;
    evmsail_receipt_table_push(&z2zE408, z2zE407, zvalue);
  }
  unit z3zE745;
  {
    z3zE745 = zoptimizzed_unit_unwrap(z2zE408);
    if (have_exception) {
      KILL(zOptimizzedUnitResult)(&z2zE408);
      goto end_block_exception_1302;
    }
  }
  KILL(zOptimizzedUnitResult)(&z2zE408);
  unit z3zE744;
  z3zE744 = zscratch_rewind(zmark);
  sail_fixed_bytes_256 z2zE411;
  {
    sail_fixed_bytes_256 z2zE409;
    z2zE409 = zacc.zbloom;
    zz5listz8z5structz0zzLogEntryz9 z2zE410;
    CREATE(zz5listz8z5structz0zzLogEntryz9)(&z2zE410);
    COPY(zz5listz8z5structz0zzLogEntryz9)(&z2zE410, zreceipt.zlogs);
    z2zE411 = zbloom_add_logs(z2zE409, z2zE410);
    KILL(zz5listz8z5structz0zzLogEntryz9)(&z2zE410);
  }
  struct zReceiptAccumulator z3zE746;
  z3zE746.zbloom = z2zE411;
  z3zE746.zcount = znext_count;
  z3zE746.zcumulative_gas_used = zcumulative;
  z8zE653 = z3zE746;
end_function_1301: ;
  return z8zE653;
end_block_exception_1302: ;
  struct zReceiptAccumulator z8zE996 = { .zbloom = fixed_bytes_256_zero(), .zcount = UINT64_C(0xdeadc0de), .zcumulative_gas_used = UINT64_C(0xdeadc0de) };
  return z8zE996;
}

sail_fixed_bytes_32 zreceipt_accumulator_root(struct zReceiptAccumulator zacc)
{
  sail_fixed_bytes_32 z8zE654;
  struct zOptimizzedHashResult z2zE406;
  CREATE(zOptimizzedHashResult)(&z2zE406);
  {
    uint64_t z2zE405;
    z2zE405 = zacc.zcount;
    evmsail_receipt_table_root(&z2zE406, z2zE405);
  }
  {
    z8zE654 = zoptimizzed_hash_unwrap(z2zE406);
    if (have_exception) {
      KILL(zOptimizzedHashResult)(&z2zE406);
      goto end_block_exception_1300;
    }
  }
  KILL(zOptimizzedHashResult)(&z2zE406);
end_function_1299: ;
  return z8zE654;
end_block_exception_1300: ;

  return fixed_bytes_32_zero();
}

