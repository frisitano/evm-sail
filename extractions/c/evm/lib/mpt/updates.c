/* Generated from sail/lib/mpt/updates.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
void zcached_account_trie_update_next(struct zoptionzIRTrieUpdatezK *z8zE511, unit z3zE1472)
{
  zNonezIRTrieUpdatezK((*(&z8zE511)), UNIT);
end_function_1922: ;
  goto end_function_4051;
end_block_exception_1923: ;
  goto end_function_4051;
end_function_4051: ;
}

struct zByteSliceFields zencode_state_account(struct zAccountInfo zinfo, sail_fixed_bytes_32 zstorage_root)
{
  struct zByteSliceFields z8zE512;
  uint64_t znonce_length;
  {
    uint64_t z2zE1489;
    z2zE1489 = zinfo.znonce;
    znonce_length = zrlp_uint_word_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE1489);
  }
  uint64_t zbalance_length;
  {
    sail_u256 z2zE1488;
    z2zE1488 = zinfo.zbalance;
    zbalance_length = zrlp_uint_word_sizze(z2zE1488);
  }
  uint64_t zcontent_len;
  {
    uint64_t z2zE1487;
    {
      uint64_t z2zE1486;
      {    z2zE1486 = (znonce_length + zbalance_length);
      }
      {    z2zE1487 = (z2zE1486 + UINT64_C(33));
      }
    }
    {    zcontent_len = (z2zE1487 + UINT64_C(33));
    }
  }
  uint64_t zstart;
  zstart = zscratch_begin(UNIT);
  unit z3zE1471;
  z3zE1471 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zcontent_len);
  uint64_t z2zE1481;
  z2zE1481 = zinfo.znonce;
  unit z3zE1470;
  z3zE1470 = zrlp_write_uint_wordzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE1481);
  sail_u256 z2zE1482;
  z2zE1482 = zinfo.zbalance;
  unit z3zE1469;
  z3zE1469 = zrlp_write_uint_word(z2zE1482);
  sail_u256 z2zE1483;
  z2zE1483 = zhash_to_word(zstorage_root);
  unit z3zE1468;
  z3zE1468 = zrlp_write_word(z2zE1483);
  sail_u256 z2zE1485;
  {
    sail_fixed_bytes_32 z2zE1484;
    z2zE1484 = zinfo.zcode_hash;
    z2zE1485 = zhash_to_word(z2zE1484);
  }
  unit z3zE1467;
  z3zE1467 = zrlp_write_word(z2zE1485);
  z8zE512 = zrlp_finish(zstart);
end_function_1920: ;
  return z8zE512;
end_block_exception_1921: ;
  struct zByteSliceFields z8zE1009 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE1009;
}

void zaccount_update(struct zTrieUpdate *z8zE513, struct zAcctEntry zentry, sail_fixed_bytes_32 zstorage_root)
{
  struct zAccount zcurrent;
  {
    struct zAcctValue z2zE1480;
    z2zE1480 = zentry.zvalue;
    zcurrent = z2zE1480.zcurr;
  }
  struct zTriePath zkey;
  {
    sail_fixed_bytes_32 z2zE1479;
    {
      sail_fixed_bytes_20 z2zE1478;
      z2zE1478 = zentry.zaddr;
      z2zE1479 = zkeccak256_address(z2zE1478);
    }
    zkey = zpath_from_hash(z2zE1479);
  }
  bool z2zE1473;
  {
    bool z2zE1472;
    {
      bool z2zE1470;
      z2zE1470 = zcurrent.zpresent;
      z2zE1472 = not(z2zE1470);
    }
    bool z3zE1464;
    if (z2zE1472) {  z3zE1464 = true;  } else {
      struct zAccountInfo z2zE1471;
      z2zE1471 = zcurrent.zinfo;
      z3zE1464 = zaccount_info_empty(z2zE1471);
    }
    z2zE1473 = z3zE1464;
  }
  if (z2zE1473) {
    struct zTrieChange z2zE1474;
    CREATE(zTrieChange)(&z2zE1474);
    zTrieDelete(&z2zE1474, UNIT);
    struct zTrieUpdate z3zE1466;
    CREATE(zTrieUpdate)(&z3zE1466);
    COPY(zTrieChange)(&((&z3zE1466)->zchange), z2zE1474);
    z3zE1466.zkey = zkey;
    COPY(zTrieUpdate)((*(&z8zE513)), z3zE1466);
    KILL(zTrieUpdate)(&z3zE1466);
    KILL(zTrieChange)(&z2zE1474);
  } else {
    struct zTrieChange z2zE1477;
    CREATE(zTrieChange)(&z2zE1477);
    {
      struct zByteSliceFields z2zE1476;
      {
        struct zAccountInfo z2zE1475;
        z2zE1475 = zcurrent.zinfo;
        z2zE1476 = zencode_state_account(z2zE1475, zstorage_root);
      }
      zTriePut(&z2zE1477, z2zE1476);
    }
    struct zTrieUpdate z3zE1465;
    CREATE(zTrieUpdate)(&z3zE1465);
    COPY(zTrieChange)(&((&z3zE1465)->zchange), z2zE1477);
    z3zE1465.zkey = zkey;
    COPY(zTrieUpdate)((*(&z8zE513)), z3zE1465);
    KILL(zTrieUpdate)(&z3zE1465);
    KILL(zTrieChange)(&z2zE1477);
  }
end_function_1918: ;
  goto end_function_4050;
end_block_exception_1919: ;
  goto end_function_4050;
end_function_4050: ;
}

bool zaccount_value_changed(struct zAcctValue zvalue)
{
  bool z8zE514;
  bool z2zE1469;
  {
    bool z2zE1433;
    {
      uint64_t z2zE1431;
      {
        struct zAccountInfo z2zE1428;
        {
          struct zAccount z2zE1427;
          z2zE1427 = zvalue.zcurr;
          z2zE1428 = z2zE1427.zinfo;
        }
        z2zE1431 = z2zE1428.znonce;
      }
      uint64_t z2zE1432;
      {
        struct zAccountInfo z2zE1430;
        {
          struct zAccount z2zE1429;
          z2zE1429 = zvalue.zorig;
          z2zE1430 = z2zE1429.zinfo;
        }
        z2zE1432 = z2zE1430.znonce;
      }
      z2zE1433 = (z2zE1431 == z2zE1432);
    }
    z2zE1469 = not(z2zE1433);
  }
  bool z3zE1463;
  if (z2zE1469) {  z3zE1463 = true;  } else {
    bool z2zE1468;
    {
      bool z2zE1440;
      {
        sail_u256 z2zE1438;
        {
          struct zAccountInfo z2zE1435;
          {
            struct zAccount z2zE1434;
            z2zE1434 = zvalue.zcurr;
            z2zE1435 = z2zE1434.zinfo;
          }
          z2zE1438 = z2zE1435.zbalance;
        }
        sail_u256 z2zE1439;
        {
          struct zAccountInfo z2zE1437;
          {
            struct zAccount z2zE1436;
            z2zE1436 = zvalue.zorig;
            z2zE1437 = z2zE1436.zinfo;
          }
          z2zE1439 = z2zE1437.zbalance;
        }
        z2zE1440 = eq_u256(z2zE1438, z2zE1439);
      }
      z2zE1468 = not(z2zE1440);
    }
    bool z3zE1462;
    if (z2zE1468) {  z3zE1462 = true;  } else {
      bool z2zE1467;
      {
        bool z2zE1447;
        {
          sail_fixed_bytes_32 z2zE1445;
          {
            struct zAccountInfo z2zE1442;
            {
              struct zAccount z2zE1441;
              z2zE1441 = zvalue.zcurr;
              z2zE1442 = z2zE1441.zinfo;
            }
            z2zE1445 = z2zE1442.zstorage_root;
          }
          sail_fixed_bytes_32 z2zE1446;
          {
            struct zAccountInfo z2zE1444;
            {
              struct zAccount z2zE1443;
              z2zE1443 = zvalue.zorig;
              z2zE1444 = z2zE1443.zinfo;
            }
            z2zE1446 = z2zE1444.zstorage_root;
          }
          z2zE1447 = eq_fixed_bytes_32(z2zE1445, z2zE1446);
        }
        z2zE1467 = not(z2zE1447);
      }
      bool z3zE1461;
      if (z2zE1467) {  z3zE1461 = true;  } else {
        bool z2zE1466;
        {
          bool z2zE1454;
          {
            sail_fixed_bytes_32 z2zE1452;
            {
              struct zAccountInfo z2zE1449;
              {
                struct zAccount z2zE1448;
                z2zE1448 = zvalue.zcurr;
                z2zE1449 = z2zE1448.zinfo;
              }
              z2zE1452 = z2zE1449.zcode_hash;
            }
            sail_fixed_bytes_32 z2zE1453;
            {
              struct zAccountInfo z2zE1451;
              {
                struct zAccount z2zE1450;
                z2zE1450 = zvalue.zorig;
                z2zE1451 = z2zE1450.zinfo;
              }
              z2zE1453 = z2zE1451.zcode_hash;
            }
            z2zE1454 = eq_fixed_bytes_32(z2zE1452, z2zE1453);
          }
          z2zE1466 = not(z2zE1454);
        }
        bool z3zE1460;
        if (z2zE1466) {  z3zE1460 = true;  } else {
          bool z2zE1465;
          {
            bool z2zE1459;
            {
              bool z2zE1457;
              {
                struct zAccount z2zE1455;
                z2zE1455 = zvalue.zcurr;
                z2zE1457 = z2zE1455.zpresent;
              }
              bool z2zE1458;
              {
                struct zAccount z2zE1456;
                z2zE1456 = zvalue.zorig;
                z2zE1458 = z2zE1456.zpresent;
              }
              z2zE1459 = eq_bool(z2zE1457, z2zE1458);
            }
            z2zE1465 = not(z2zE1459);
          }
          bool z3zE1459;
          if (z2zE1465) {  z3zE1459 = true;  } else {
            bool z2zE1464;
            {
              bool z2zE1462;
              {
                struct zAccount z2zE1460;
                z2zE1460 = zvalue.zcurr;
                z2zE1462 = z2zE1460.zstorage_cleared;
              }
              bool z2zE1463;
              {
                struct zAccount z2zE1461;
                z2zE1461 = zvalue.zorig;
                z2zE1463 = z2zE1461.zstorage_cleared;
              }
              z2zE1464 = eq_bool(z2zE1462, z2zE1463);
            }
            z3zE1459 = not(z2zE1464);
          }
          z3zE1460 = z3zE1459;
        }
        z3zE1461 = z3zE1460;
      }
      z3zE1462 = z3zE1461;
    }
    z3zE1463 = z3zE1462;
  }
  z8zE514 = z3zE1463;
end_function_1916: ;
  return z8zE514;
end_block_exception_1917: ;

  return false;
}

bool zstorage_value_changed(struct zStorageValue zvalue)
{
  bool z8zE515;
  bool z2zE1426;
  {
    sail_u256 z2zE1424;
    z2zE1424 = zvalue.zcurr;
    sail_u256 z2zE1425;
    z2zE1425 = zvalue.zorig;
    z2zE1426 = eq_u256(z2zE1424, z2zE1425);
  }
  z8zE515 = not(z2zE1426);
end_function_1914: ;
  return z8zE515;
end_block_exception_1915: ;

  return false;
}

void znext_changed_storage_entry(struct zoptionzIRStorageEntryzK *z8zE516, sail_fixed_bytes_20 zaddr)
{
  bool zsearching;
  zsearching = true;
  struct zoptionzIRStorageEntryzK zresult;
  CREATE(zoptionzIRStorageEntryzK)(&zresult);
  zNonezIRStorageEntryzK(&zresult, UNIT);
  bool z3zE1456;
  unit z3zE1457;
while_1907: ;
  {
    z3zE1456 = zsearching;
    if (!(z3zE1456)) goto wend_1908;
    struct zoptionzIRStorageEntryzK z2zE1421;
    CREATE(zoptionzIRStorageEntryzK)(&z2zE1421);
    storage_block_iter_next(&z2zE1421, zaddr);
    unit z3zE1452;
    {
      if (z2zE1421.kind != Kind_zSomezIRStorageEntryzK) goto case_1911;
      struct zStorageEntry zentry;
      zentry = z2zE1421.variants.zSomezIRStorageEntryzK;
      bool z2zE1423;
      {
        struct zStorageValue z2zE1422;
        z2zE1422 = zentry.zvalue;
        z2zE1423 = zstorage_value_changed(z2zE1422);
      }
      if (z2zE1423) {
        zSomezIRStorageEntryzK(&zresult, zentry);
        unit z3zE1455;
        z3zE1455 = UNIT;
        zsearching = false;
        z3zE1452 = UNIT;
      } else {  z3zE1452 = UNIT;  }
      goto finish_match_1909;
    }
  case_1911: ;
    {
      /* complete */
      zsearching = false;
      z3zE1452 = UNIT;
      goto finish_match_1909;
    }
  case_1910: ;
  finish_match_1909: ;
    z3zE1457 = z3zE1452;
    KILL(zoptionzIRStorageEntryzK)(&z2zE1421);
    goto while_1907;
  }
wend_1908: ;
  unit z3zE1458;
  z3zE1458 = UNIT;
  COPY(zoptionzIRStorageEntryzK)((*(&z8zE516)), zresult);
  KILL(zoptionzIRStorageEntryzK)(&zresult);
end_function_1912: ;
  goto end_function_4049;
end_block_exception_1913: ;
  goto end_function_4049;
end_function_4049: ;
}

void zaccount_trie_update(struct ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9 *z8zE517, struct zAcctEntry zentry)
{
  sail_fixed_bytes_20 z2zE1413;
  z2zE1413 = zentry.zaddr;
  unit z3zE1446;
  z3zE1446 = storage_block_iter_begin(z2zE1413);
  bool zstorage_changed;
  {
    struct zoptionzIRStorageEntryzK z2zE1420;
    CREATE(zoptionzIRStorageEntryzK)(&z2zE1420);
    {
      sail_fixed_bytes_20 z2zE1419;
      z2zE1419 = zentry.zaddr;
      znext_changed_storage_entry(&z2zE1420, z2zE1419);
    }
    bool z3zE1447;
    {
      if (z2zE1420.kind != Kind_zSomezIRStorageEntryzK) goto case_1904;
      struct zStorageEntry z_storage_entry;
      z_storage_entry = z2zE1420.variants.zSomezIRStorageEntryzK;
      z3zE1447 = true;
      goto finish_match_1902;
    }
  case_1904: ;
    {
      /* complete */
      z3zE1447 = false;
      goto finish_match_1902;
    }
  case_1903: ;
  finish_match_1902: ;
    zstorage_changed = z3zE1447;
    KILL(zoptionzIRStorageEntryzK)(&z2zE1420);
  }
  sail_fixed_bytes_32 zstorage_root;
  {
    sail_fixed_bytes_20 z2zE1418;
    z2zE1418 = zentry.zaddr;
    zstorage_root = acct_post_storage_root_read(z2zE1418);
  }
  struct zTrieUpdate z2zE1416;
  CREATE(zTrieUpdate)(&z2zE1416);
  zaccount_update(&z2zE1416, zentry, zstorage_root);
  bool z2zE1417;
  {
    bool z2zE1415;
    {
      struct zAcctValue z2zE1414;
      z2zE1414 = zentry.zvalue;
      z2zE1415 = zaccount_value_changed(z2zE1414);
    }
    bool z3zE1450;
    if (z2zE1415) {  z3zE1450 = true;  } else {  z3zE1450 = zstorage_changed;  }
    z2zE1417 = z3zE1450;
  }
  struct ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9 z3zE1451;
  CREATE(ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9)(&z3zE1451);
  COPY(zTrieUpdate)(&((&z3zE1451)->ztup0), z2zE1416);
  z3zE1451.ztup1 = z2zE1417;
  COPY(ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9)((*(&z8zE517)), z3zE1451);
  KILL(ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9)(&z3zE1451);
  KILL(zTrieUpdate)(&z2zE1416);
end_function_1905: ;
  goto end_function_4048;
end_block_exception_1906: ;
  goto end_function_4048;
end_function_4048: ;
}

void znext_changed_account_trie_update(struct zoptionzIRTrieUpdatezK *z8zE518, unit z3zE1436)
{
  bool zsearching;
  zsearching = true;
  struct zoptionzIRTrieUpdatezK zresult;
  CREATE(zoptionzIRTrieUpdatezK)(&zresult);
  zNonezIRTrieUpdatezK(&zresult, UNIT);
  bool z3zE1443;
  unit z3zE1444;
while_1893: ;
  {
    z3zE1443 = zsearching;
    if (!(z3zE1443)) goto wend_1894;
    struct zoptionzIRAcctEntryzK z2zE1411;
    CREATE(zoptionzIRAcctEntryzK)(&z2zE1411);
    acct_block_iter_next(&z2zE1411, UNIT);
    unit z3zE1437;
    {
      if (z2zE1411.kind != Kind_zSomezIRAcctEntryzK) goto case_1897;
      struct zAcctEntry zentry;
      zentry = z2zE1411.variants.zSomezIRAcctEntryzK;
      struct ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9 z2zE1412;
      CREATE(ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9)(&z2zE1412);
      zaccount_trie_update(&z2zE1412, zentry);
      unit z3zE1439;
      {
        struct zTrieUpdate zupdate;
        CREATE(zTrieUpdate)(&zupdate);
        COPY(zTrieUpdate)(&zupdate, z2zE1412.ztup0);
        bool zchanged;
        zchanged = z2zE1412.ztup1;
        if (zchanged) {
          zSomezIRTrieUpdatezK(&zresult, zupdate);
          unit z3zE1441;
          z3zE1441 = UNIT;
          zsearching = false;
          z3zE1439 = UNIT;
        } else {  z3zE1439 = UNIT;  }
        KILL(zTrieUpdate)(&zupdate);
        goto finish_match_1898;
      }
    case_1899: ;
      sail_match_failure("next_changed_account_trie_update");
    finish_match_1898: ;
      z3zE1437 = z3zE1439;
      KILL(ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9)(&z2zE1412);
      goto finish_match_1895;
    }
  case_1897: ;
    {
      /* complete */
      zsearching = false;
      z3zE1437 = UNIT;
      goto finish_match_1895;
    }
  case_1896: ;
  finish_match_1895: ;
    z3zE1444 = z3zE1437;
    KILL(zoptionzIRAcctEntryzK)(&z2zE1411);
    goto while_1893;
  }
wend_1894: ;
  unit z3zE1445;
  z3zE1445 = UNIT;
  COPY(zoptionzIRTrieUpdatezK)((*(&z8zE518)), zresult);
  KILL(zoptionzIRTrieUpdatezK)(&zresult);
end_function_1900: ;
  goto end_function_4047;
end_block_exception_1901: ;
  goto end_function_4047;
end_function_4047: ;
}

struct zByteSliceFields zencode_storage_value(sail_u256 zvalue)
{
  struct zByteSliceFields z8zE519;
  uint64_t zencoded_len;
  zencoded_len = zrlp_uint_word_sizze(zvalue);
  uint64_t zstart;
  zstart = zscratch_begin(UNIT);
  unit z3zE1435;
  z3zE1435 = zrlp_write_uint_word(zvalue);
  z8zE519 = zrlp_finish(zstart);
end_function_1891: ;
  return z8zE519;
end_block_exception_1892: ;
  struct zByteSliceFields z8zE1010 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE1010;
}

void zstorage_update(struct zTrieUpdate *z8zE520, struct zStorageEntry zentry)
{
  struct zTriePath zkey;
  {
    sail_fixed_bytes_32 z2zE1410;
    {
      sail_u256 z2zE1409;
      {
        struct zStorageKey z2zE1408;
        z2zE1408 = zentry.zkey;
        z2zE1409 = z2zE1408.zslot;
      }
      z2zE1410 = zkeccak256_word(z2zE1409);
    }
    zkey = zpath_from_hash(z2zE1410);
  }
  struct zTrieChange zchange;
  CREATE(zTrieChange)(&zchange);
  {
    bool z2zE1404;
    {
      sail_u256 z2zE1403;
      {
        struct zStorageValue z2zE1402;
        z2zE1402 = zentry.zvalue;
        z2zE1403 = z2zE1402.zcurr;
      }
      z2zE1404 = zword_is_zzero(z2zE1403);
    }
    if (z2zE1404) {  zTrieDelete(&zchange, UNIT);  } else {
      struct zByteSliceFields z2zE1407;
      {
        sail_u256 z2zE1406;
        {
          struct zStorageValue z2zE1405;
          z2zE1405 = zentry.zvalue;
          z2zE1406 = z2zE1405.zcurr;
        }
        z2zE1407 = zencode_storage_value(z2zE1406);
      }
      zTriePut(&zchange, z2zE1407);
    }
  }
  struct zTrieUpdate z3zE1434;
  CREATE(zTrieUpdate)(&z3zE1434);
  COPY(zTrieChange)(&((&z3zE1434)->zchange), zchange);
  z3zE1434.zkey = zkey;
  COPY(zTrieUpdate)((*(&z8zE520)), z3zE1434);
  KILL(zTrieUpdate)(&z3zE1434);
  KILL(zTrieChange)(&zchange);
end_function_1889: ;
  goto end_function_4046;
end_block_exception_1890: ;
  goto end_function_4046;
end_function_4046: ;
}

void znext_storage_trie_update(struct zoptionzIRTrieUpdatezK *z8zE521, sail_fixed_bytes_20 zaddr)
{
  struct zoptionzIRStorageEntryzK z2zE1400;
  CREATE(zoptionzIRStorageEntryzK)(&z2zE1400);
  znext_changed_storage_entry(&z2zE1400, zaddr);
  struct zoptionzIRTrieUpdatezK z3zE1431;
  CREATE(zoptionzIRTrieUpdatezK)(&z3zE1431);
  {
    if (z2zE1400.kind != Kind_zSomezIRStorageEntryzK) goto case_1886;
    struct zStorageEntry zentry;
    zentry = z2zE1400.variants.zSomezIRStorageEntryzK;
    struct zTrieUpdate z2zE1401;
    CREATE(zTrieUpdate)(&z2zE1401);
    zstorage_update(&z2zE1401, zentry);
    zSomezIRTrieUpdatezK(&z3zE1431, z2zE1401);
    KILL(zTrieUpdate)(&z2zE1401);
    goto finish_match_1884;
  }
case_1886: ;
  {
    /* complete */
    zNonezIRTrieUpdatezK(&z3zE1431, UNIT);
    goto finish_match_1884;
  }
case_1885: ;
finish_match_1884: ;
  COPY(zoptionzIRTrieUpdatezK)((*(&z8zE521)), z3zE1431);
  KILL(zoptionzIRTrieUpdatezK)(&z3zE1431);
  KILL(zoptionzIRStorageEntryzK)(&z2zE1400);
end_function_1887: ;
  goto end_function_4045;
end_block_exception_1888: ;
  goto end_function_4045;
end_function_4045: ;
}

void ztrie_update_source_next(struct zoptionzIRTrieUpdatezK *z8zE522, struct zTrieUpdateSource zsource)
{
  struct zoptionzIRTrieUpdatezK z3zE1427;
  CREATE(zoptionzIRTrieUpdatezK)(&z3zE1427);
  {
    if (zsource.kind != Kind_zStorageTrieUpdates) goto case_1881;
    sail_fixed_bytes_20 zaddr;
    zaddr = zsource.variants.zStorageTrieUpdates;
    znext_storage_trie_update(&z3zE1427, zaddr);
    goto finish_match_1878;
  }
case_1881: ;
  {
    if (zsource.kind != Kind_zChangedAccountTrieUpdates) goto case_1880;
    znext_changed_account_trie_update(&z3zE1427, UNIT);
    goto finish_match_1878;
  }
case_1880: ;
  {
    /* complete */
    zcached_account_trie_update_next(&z3zE1427, UNIT);
    goto finish_match_1878;
  }
case_1879: ;
finish_match_1878: ;
  COPY(zoptionzIRTrieUpdatezK)((*(&z8zE522)), z3zE1427);
  KILL(zoptionzIRTrieUpdatezK)(&z3zE1427);
end_function_1882: ;
  goto end_function_4044;
end_block_exception_1883: ;
  goto end_function_4044;
end_function_4044: ;
}

void ztrie_updates_begin(struct zTrieUpdateCursor *z8zE523, struct zTrieUpdateSource zsource)
{
  struct zoptionzIRTrieUpdatezK z2zE1399;
  CREATE(zoptionzIRTrieUpdatezK)(&z2zE1399);
  ztrie_update_source_next(&z2zE1399, zsource);
  struct zTrieUpdateCursor z3zE1426;
  CREATE(zTrieUpdateCursor)(&z3zE1426);
  COPY(zoptionzIRTrieUpdatezK)(&((&z3zE1426)->zpending), z2zE1399);
  COPY(zTrieUpdateSource)(&((&z3zE1426)->zsource), zsource);
  COPY(zTrieUpdateCursor)((*(&z8zE523)), z3zE1426);
  KILL(zTrieUpdateCursor)(&z3zE1426);
  KILL(zoptionzIRTrieUpdatezK)(&z2zE1399);
end_function_1876: ;
  goto end_function_4043;
end_block_exception_1877: ;
  goto end_function_4043;
end_function_4043: ;
}

bool zupdates_empty(struct zTrieUpdateCursor zupdates)
{
  bool z8zE524;
  struct zoptionzIRTrieUpdatezK z2zE1398;
  CREATE(zoptionzIRTrieUpdatezK)(&z2zE1398);
  COPY(zoptionzIRTrieUpdatezK)(&z2zE1398, zupdates.zpending);
  bool z3zE1423;
  {
    if (z2zE1398.kind != Kind_zNonezIRTrieUpdatezK) goto case_1873;
    z3zE1423 = true;
    goto finish_match_1871;
  }
case_1873: ;
  {
    /* complete */
    z3zE1423 = false;
    goto finish_match_1871;
  }
case_1872: ;
finish_match_1871: ;
  z8zE524 = z3zE1423;
  KILL(zoptionzIRTrieUpdatezK)(&z2zE1398);
end_function_1874: ;
  return z8zE524;
end_block_exception_1875: ;

  return false;
}

void ztrie_updates_advance(struct zTrieUpdateCursor *z8zE525, struct zTrieUpdateCursor zupdates)
{
  struct zTrieUpdateSource z2zE1395;
  CREATE(zTrieUpdateSource)(&z2zE1395);
  COPY(zTrieUpdateSource)(&z2zE1395, zupdates.zsource);
  struct zoptionzIRTrieUpdatezK z2zE1397;
  CREATE(zoptionzIRTrieUpdatezK)(&z2zE1397);
  {
    struct zTrieUpdateSource z2zE1396;
    CREATE(zTrieUpdateSource)(&z2zE1396);
    COPY(zTrieUpdateSource)(&z2zE1396, zupdates.zsource);
    ztrie_update_source_next(&z2zE1397, z2zE1396);
    KILL(zTrieUpdateSource)(&z2zE1396);
  }
  struct zTrieUpdateCursor z3zE1422;
  CREATE(zTrieUpdateCursor)(&z3zE1422);
  COPY(zoptionzIRTrieUpdatezK)(&((&z3zE1422)->zpending), z2zE1397);
  COPY(zTrieUpdateSource)(&((&z3zE1422)->zsource), z2zE1395);
  COPY(zTrieUpdateCursor)((*(&z8zE525)), z3zE1422);
  KILL(zTrieUpdateCursor)(&z3zE1422);
  KILL(zoptionzIRTrieUpdatezK)(&z2zE1397);
  KILL(zTrieUpdateSource)(&z2zE1395);
end_function_1869: ;
  goto end_function_4042;
end_block_exception_1870: ;
  goto end_function_4042;
end_function_4042: ;
}

bool znext_update_under(struct zTrieUpdateCursor zupdates, struct zTriePath zprefix)
{
  bool z8zE526;
  struct zoptionzIRTrieUpdatezK z2zE1393;
  CREATE(zoptionzIRTrieUpdatezK)(&z2zE1393);
  COPY(zoptionzIRTrieUpdatezK)(&z2zE1393, zupdates.zpending);
  bool z3zE1419;
  {
    if (z2zE1393.kind != Kind_zSomezIRTrieUpdatezK) goto case_1866;
    struct zTrieUpdate zupdate;
    CREATE(zTrieUpdate)(&zupdate);
    COPY(zTrieUpdate)(&zupdate, z2zE1393.variants.zSomezIRTrieUpdatezK);
    struct zTriePath z2zE1394;
    z2zE1394 = zupdate.zkey;
    {
      z3zE1419 = zpath_prefix_of(zprefix, z2zE1394);
      if (have_exception) {
        KILL(zoptionzIRTrieUpdatezK)(&z2zE1393);
        KILL(zTrieUpdate)(&zupdate);
        goto end_block_exception_1868;
      }
    }
    KILL(zTrieUpdate)(&zupdate);
    goto finish_match_1864;
  }
case_1866: ;
  {
    /* complete */
    z3zE1419 = false;
    goto finish_match_1864;
  }
case_1865: ;
finish_match_1864: ;
  z8zE526 = z3zE1419;
  KILL(zoptionzIRTrieUpdatezK)(&z2zE1393);
end_function_1867: ;
  return z8zE526;
end_block_exception_1868: ;

  return false;
}

void zitem_leaf(struct zTrieItem *z8zE527, struct zTriePath zpath, struct zByteSliceFields zvalue)
{
  struct zTrieItemValue z2zE1392;
  CREATE(zTrieItemValue)(&z2zE1392);
  zLeafItem(&z2zE1392, zvalue);
  struct zTrieItem z3zE1418;
  CREATE(zTrieItem)(&z3zE1418);
  z3zE1418.zpath = zpath;
  COPY(zTrieItemValue)(&((&z3zE1418)->zvalue), z2zE1392);
  COPY(zTrieItem)((*(&z8zE527)), z3zE1418);
  KILL(zTrieItem)(&z3zE1418);
  KILL(zTrieItemValue)(&z2zE1392);
end_function_1862: ;
  goto end_function_4041;
end_block_exception_1863: ;
  goto end_function_4041;
end_function_4041: ;
}

void zitem_branch(struct zTrieItem *z8zE528, struct zTriePath zpath, struct zNodeRef zchildref)
{
  struct zTrieItemValue z2zE1391;
  CREATE(zTrieItemValue)(&z2zE1391);
  zBranchItem(&z2zE1391, zchildref);
  struct zTrieItem z3zE1417;
  CREATE(zTrieItem)(&z3zE1417);
  z3zE1417.zpath = zpath;
  COPY(zTrieItemValue)(&((&z3zE1417)->zvalue), z2zE1391);
  COPY(zTrieItem)((*(&z8zE528)), z3zE1417);
  KILL(zTrieItem)(&z3zE1417);
  KILL(zTrieItemValue)(&z2zE1391);
end_function_1860: ;
  goto end_function_4040;
end_block_exception_1861: ;
  goto end_function_4040;
end_function_4040: ;
}

void zitem_subtree(struct zTrieItem *z8zE529, struct zTriePath zpath, struct zNodeRef zchildref)
{
  struct zTrieItemValue z2zE1390;
  CREATE(zTrieItemValue)(&z2zE1390);
  zSubtreeItem(&z2zE1390, zchildref);
  struct zTrieItem z3zE1416;
  CREATE(zTrieItem)(&z3zE1416);
  z3zE1416.zpath = zpath;
  COPY(zTrieItemValue)(&((&z3zE1416)->zvalue), z2zE1390);
  COPY(zTrieItem)((*(&z8zE529)), z3zE1416);
  KILL(zTrieItem)(&z3zE1416);
  KILL(zTrieItemValue)(&z2zE1390);
end_function_1858: ;
  goto end_function_4039;
end_block_exception_1859: ;
  goto end_function_4039;
end_function_4039: ;
}

void zitem_ref(struct zNodeRef *z8zE530, struct zTrieItem zit, uint64_t zdepth)
{
  struct zTriePath zsuffix;
  {
    struct zTriePath z2zE1389;
    z2zE1389 = zit.zpath;
    {
      zsuffix = zpath_drop(z2zE1389, zdepth);
      if (have_exception) {  goto end_block_exception_1857;  }
    }
  }
  struct zTrieItemValue z2zE1381;
  CREATE(zTrieItemValue)(&z2zE1381);
  COPY(zTrieItemValue)(&z2zE1381, zit.zvalue);
  struct zNodeRef z3zE1408;
  CREATE(zNodeRef)(&z3zE1408);
  {
    if (z2zE1381.kind != Kind_zLeafItem) goto case_1855;
    struct zByteSliceFields zvalue;
    zvalue = z2zE1381.variants.zLeafItem;
    {
      zleaf_child_ref(&z3zE1408, zsuffix, zvalue);
      if (have_exception) {
        KILL(zTrieItemValue)(&z2zE1381);
        KILL(zNodeRef)(&z3zE1408);
        goto end_block_exception_1857;
      }
    }
    goto finish_match_1849;
  }
case_1855: ;
  {
    if (z2zE1381.kind != Kind_zBranchItem) goto case_1854;
    struct zNodeRef zsubref;
    CREATE(zNodeRef)(&zsubref);
    COPY(zNodeRef)(&zsubref, z2zE1381.variants.zBranchItem);
    bool z2zE1383;
    {
      uint64_t z2zE1382;
      z2zE1382 = zpath_len(zsuffix);
      z2zE1383 = (z2zE1382 == UINT64_C(0));
    }
    if (z2zE1383) {  COPY(zNodeRef)(&z3zE1408, zsubref);  } else {
      {
        zmerge_ext_ref(&z3zE1408, zsuffix, zsubref);
        if (have_exception) {
          KILL(zTrieItemValue)(&z2zE1381);
          KILL(zNodeRef)(&z3zE1408);
          KILL(zNodeRef)(&zsubref);
          goto end_block_exception_1857;
        }
      }
    }
    KILL(zNodeRef)(&zsubref);
    goto finish_match_1849;
  }
case_1854: ;
  {
    /* complete */
    struct zNodeRef z3zE1415;
    CREATE(zNodeRef)(&z3zE1415);
    COPY(zNodeRef)(&z3zE1415, z2zE1381.variants.zSubtreeItem);
    bool z2zE1385;
    {
      uint64_t z2zE1384;
      z2zE1384 = zpath_len(zsuffix);
      z2zE1385 = (z2zE1384 == UINT64_C(0));
    }
    if (z2zE1385) {  COPY(zNodeRef)(&z3zE1408, z3zE1415);  } else {
      struct zNodeRef z3zE1410;
      CREATE(zNodeRef)(&z3zE1410);
      {
        if (z3zE1415.kind != Kind_zHashRef) goto case_1853;
        sail_fixed_bytes_32 zh;
        zh = z3zE1415.variants.zHashRef;
        struct zByteSliceFields znode;
        znode = znode_db_lookup(zh);
        bool z2zE1387;
        {
          uint64_t z2zE1386;
          z2zE1386 = znode.zlen;
          z2zE1387 = (z2zE1386 == UINT64_C(0));
        }
        if (z2zE1387) {
          struct zexception z2zE1388;
          CREATE(zexception)(&z2zE1388);
          zInvalidBlock(&z2zE1388, zWitnessDeficient);
          COPY(zexception)(current_exception, z2zE1388);
          have_exception = true;
          COPY(sail_string)(throw_location, "sail/lib/mpt/updates.sail:117.24-117.60");
          KILL(zTrieItemValue)(&z2zE1381);
          KILL(zNodeRef)(&z3zE1408);
          KILL(zNodeRef)(&z3zE1415);
          KILL(zNodeRef)(&z3zE1410);
          KILL(zexception)(&z2zE1388);
          goto end_block_exception_1857;
          /* unreachable after throw */
          KILL(zexception)(&z2zE1388);
        } else {
          {
            zmerge_ext_node(&z3zE1410, zsuffix, znode);
            if (have_exception) {
              KILL(zTrieItemValue)(&z2zE1381);
              KILL(zNodeRef)(&z3zE1408);
              KILL(zNodeRef)(&z3zE1415);
              KILL(zNodeRef)(&z3zE1410);
              goto end_block_exception_1857;
            }
          }
        }
        goto finish_match_1851;
      }
    case_1853: ;
      {
        {
          zmerge_ext_ref(&z3zE1410, zsuffix, z3zE1415);
          if (have_exception) {
            KILL(zTrieItemValue)(&z2zE1381);
            KILL(zNodeRef)(&z3zE1408);
            KILL(zNodeRef)(&z3zE1415);
            KILL(zNodeRef)(&z3zE1410);
            goto end_block_exception_1857;
          }
        }
        goto finish_match_1851;
      }
    case_1852: ;
    finish_match_1851: ;
      COPY(zNodeRef)(&z3zE1408, z3zE1410);
      KILL(zNodeRef)(&z3zE1410);
    }
    KILL(zNodeRef)(&z3zE1415);
    goto finish_match_1849;
  }
case_1850: ;
finish_match_1849: ;
  COPY(zNodeRef)((*(&z8zE530)), z3zE1408);
  KILL(zNodeRef)(&z3zE1408);
  KILL(zTrieItemValue)(&z2zE1381);
end_function_1856: ;
  goto end_function_4038;
end_block_exception_1857: ;
  goto end_function_4038;
end_function_4038: ;
}

void zempty_trie_branch_frame(struct zTrieBranchFrame *z8zE531, uint64_t zdepth)
{
  zz5vecz8z5unionz0zzNodeRefz9 z2zE1380;
  CREATE(zz5vecz8z5unionz0zzNodeRefz9)(&z2zE1380);
  {
    struct zNodeRef z2zE1379;
    CREATE(zNodeRef)(&z2zE1379);
    zEmptyRef(&z2zE1379, UNIT);
    fast_unsigned_vector_init_zz5vecz8z5unionz0zzNodeRefz9(&z2zE1380, UINT64_C(16), z2zE1379);
    KILL(zNodeRef)(&z2zE1379);
  }
  struct zTrieBranchFrame z3zE1407;
  CREATE(zTrieBranchFrame)(&z3zE1407);
  COPY(zz5vecz8z5unionz0zzNodeRefz9)(&((&z3zE1407)->zchildren), z2zE1380);
  z3zE1407.zdepth = zdepth;
  z3zE1407.zmask = UINT64_C(0x0000);
  COPY(zTrieBranchFrame)((*(&z8zE531)), z3zE1407);
  KILL(zTrieBranchFrame)(&z3zE1407);
  KILL(zz5vecz8z5unionz0zzNodeRefz9)(&z2zE1380);
end_function_1847: ;
  goto end_function_4037;
end_block_exception_1848: ;
  goto end_function_4037;
end_function_4037: ;
}

void ztrie_builder_empty(struct zTrieBuilder *z8zE532, unit z3zE1404)
{
  struct zNodeRef z2zE1378;
  CREATE(zNodeRef)(&z2zE1378);
  zEmptyRef(&z2zE1378, UNIT);
  struct zTrieBuilder z3zE1405;
  CREATE(zTrieBuilder)(&z3zE1405);
  z3zE1405.zcomplete = false;
  zz5listz8z5structz0zzTrieBranchFramez9 z3zE1406;
  CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z3zE1406);
  COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&((&z3zE1405)->zframes), z3zE1406);
  KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z3zE1406);
  COPY(zNodeRef)(&((&z3zE1405)->zroot), z2zE1378);
  COPY(zTrieBuilder)((*(&z8zE532)), z3zE1405);
  KILL(zTrieBuilder)(&z3zE1405);
  KILL(zNodeRef)(&z2zE1378);
end_function_1845: ;
  goto end_function_4036;
end_block_exception_1846: ;
  goto end_function_4036;
end_function_4036: ;
}

void ztrie_builder_push(struct zTrieBuilder *z8zE533, struct zTrieBuilder zbuilder, uint64_t zdepth)
{
  zz5listz8z5structz0zzTrieBranchFramez9 z2zE1375;
  CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1375);
  {
    struct zTrieBranchFrame z2zE1373;
    CREATE(zTrieBranchFrame)(&z2zE1373);
    zempty_trie_branch_frame(&z2zE1373, zdepth);
    zz5listz8z5structz0zzTrieBranchFramez9 z2zE1374;
    CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1374);
    COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1374, zbuilder.zframes);
    zconsz3z5structz0zzTrieBranchFrame(&z2zE1375, z2zE1373, z2zE1374);
    KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1374);
    KILL(zTrieBranchFrame)(&z2zE1373);
  }
  struct zNodeRef z2zE1376;
  CREATE(zNodeRef)(&z2zE1376);
  COPY(zNodeRef)(&z2zE1376, zbuilder.zroot);
  bool z2zE1377;
  z2zE1377 = zbuilder.zcomplete;
  struct zTrieBuilder z3zE1403;
  CREATE(zTrieBuilder)(&z3zE1403);
  z3zE1403.zcomplete = z2zE1377;
  COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&((&z3zE1403)->zframes), z2zE1375);
  COPY(zNodeRef)(&((&z3zE1403)->zroot), z2zE1376);
  COPY(zTrieBuilder)((*(&z8zE533)), z3zE1403);
  KILL(zTrieBuilder)(&z3zE1403);
  KILL(zNodeRef)(&z2zE1376);
  KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1375);
end_function_1843: ;
  goto end_function_4035;
end_block_exception_1844: ;
  goto end_function_4035;
end_function_4035: ;
}

void ztrie_builder_attach(struct zTrieBuilder *z8zE534, struct zTrieBuilder zbuilder, struct zTriePath zpath, struct zNodeRef zchild)
{
  zz5listz8z5structz0zzTrieBranchFramez9 z2zE1358;
  CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1358);
  COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1358, zbuilder.zframes);
  struct zTrieBuilder z3zE1394;
  CREATE(zTrieBuilder)(&z3zE1394);
  {
    if (!((z2zE1358 == NULL))) goto case_1840;
    struct zexception z2zE1359;
    CREATE(zexception)(&z2zE1359);
    zInvalidBlock(&z2zE1359, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE1359);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/updates.sail:181.16-181.52");
    KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1358);
    KILL(zTrieBuilder)(&z3zE1394);
    KILL(zexception)(&z2zE1359);
    goto end_block_exception_1842;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1359);
    goto finish_match_1838;
  }
case_1840: ;
  {
    /* complete */
    struct zTrieBranchFrame zcurrent;
    CREATE(zTrieBranchFrame)(&zcurrent);
    COPY(zTrieBranchFrame)(&zcurrent, (*z2zE1358).hd);
    zz5listz8z5structz0zzTrieBranchFramez9 zrest;
    CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&zrest);
    COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&zrest, (*z2zE1358).tl);
    struct zTrieBranchFrame zframe;
    CREATE(zTrieBranchFrame)(&zframe);
    COPY(zTrieBranchFrame)(&zframe, zcurrent);
    uint64_t zdepth;
    zdepth = zframe.zdepth;
    bool z2zE1361;
    {
      uint64_t z2zE1360;
      z2zE1360 = zpath_len(zpath);
      z2zE1361 = (!(zdepth < z2zE1360));
    }
    unit z3zE1395;
    if (z2zE1361) {
      struct zexception z2zE1362;
      CREATE(zexception)(&z2zE1362);
      zInvalidBlock(&z2zE1362, zWitnessDeficient);
      COPY(zexception)(current_exception, z2zE1362);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/lib/mpt/updates.sail:186.16-186.52");
      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1358);
      KILL(zTrieBuilder)(&z3zE1394);
      KILL(zTrieBranchFrame)(&zcurrent);
      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&zrest);
      KILL(zTrieBranchFrame)(&zframe);
      KILL(zexception)(&z2zE1362);
      goto end_block_exception_1842;
      /* unreachable after throw */
      KILL(zexception)(&z2zE1362);
    } else {  z3zE1395 = UNIT;  }
    uint64_t zchild_index;
    zchild_index = zpath_nibble(zpath, zdepth);
    bool z2zE1364;
    {
      uint64_t z2zE1363;
      z2zE1363 = zframe.zmask;
      z2zE1364 = zbranch_mask_has(z2zE1363, zchild_index);
    }
    unit z3zE1399;
    if (z2zE1364) {
      struct zexception z2zE1365;
      CREATE(zexception)(&z2zE1365);
      zInvalidBlock(&z2zE1365, zWitnessDeficient);
      COPY(zexception)(current_exception, z2zE1365);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/lib/mpt/updates.sail:190.16-190.52");
      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1358);
      KILL(zTrieBuilder)(&z3zE1394);
      KILL(zTrieBranchFrame)(&zcurrent);
      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&zrest);
      KILL(zTrieBranchFrame)(&zframe);
      KILL(zexception)(&z2zE1365);
      goto end_block_exception_1842;
      /* unreachable after throw */
      KILL(zexception)(&z2zE1365);
    } else {  z3zE1399 = UNIT;  }
    uint64_t z2zE1366;
    z2zE1366 = zframe.zmask;
    zframe.zmask = zbranch_mask_set(z2zE1366, zchild_index);
    unit z3zE1398;
    z3zE1398 = UNIT;
    zz5vecz8z5unionz0zzNodeRefz9 z2zE1369;
    CREATE(zz5vecz8z5unionz0zzNodeRefz9)(&z2zE1369);
    {
      zz5vecz8z5unionz0zzNodeRefz9 z2zE1367;
      CREATE(zz5vecz8z5unionz0zzNodeRefz9)(&z2zE1367);
      COPY(zz5vecz8z5unionz0zzNodeRefz9)(&z2zE1367, zframe.zchildren);
      uint64_t z2zE1368;
      z2zE1368 = ((uint64_t) zchild_index);
      fast_unsigned_vector_update_zz5vecz8z5unionz0zzNodeRefz9(&z2zE1369, z2zE1367, z2zE1368, zchild);
      KILL(zz5vecz8z5unionz0zzNodeRefz9)(&z2zE1367);
    }
    COPY(zz5vecz8z5unionz0zzNodeRefz9)(&((&zframe)->zchildren), z2zE1369);
    unit z3zE1397;
    z3zE1397 = UNIT;
    zz5listz8z5structz0zzTrieBranchFramez9 z2zE1370;
    CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1370);
    zconsz3z5structz0zzTrieBranchFrame(&z2zE1370, zframe, zrest);
    struct zNodeRef z2zE1371;
    CREATE(zNodeRef)(&z2zE1371);
    COPY(zNodeRef)(&z2zE1371, zbuilder.zroot);
    bool z2zE1372;
    z2zE1372 = zbuilder.zcomplete;
    z3zE1394.zcomplete = z2zE1372;
    COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&((&z3zE1394)->zframes), z2zE1370);
    COPY(zNodeRef)(&((&z3zE1394)->zroot), z2zE1371);
    goto finish_match_1838;
  }
case_1839: ;
finish_match_1838: ;
  COPY(zTrieBuilder)((*(&z8zE534)), z3zE1394);
  KILL(zTrieBuilder)(&z3zE1394);
  KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1358);
end_function_1841: ;
  goto end_function_4034;
end_block_exception_1842: ;
  goto end_function_4034;
end_function_4034: ;
}

void ztrie_builder_pop(struct ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9 *z8zE535, struct zTrieBuilder zbuilder)
{
  zz5listz8z5structz0zzTrieBranchFramez9 z2zE1353;
  CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1353);
  COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1353, zbuilder.zframes);
  struct ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9 z3zE1389;
  CREATE(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z3zE1389);
  {
    if (!((z2zE1353 == NULL))) goto case_1835;
    struct zexception z2zE1354;
    CREATE(zexception)(&z2zE1354);
    zInvalidBlock(&z2zE1354, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE1354);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/updates.sail:201.16-201.52");
    KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1353);
    KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z3zE1389);
    KILL(zexception)(&z2zE1354);
    goto end_block_exception_1837;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1354);
    goto finish_match_1833;
  }
case_1835: ;
  {
    /* complete */
    struct zTrieBranchFrame zframe;
    CREATE(zTrieBranchFrame)(&zframe);
    COPY(zTrieBranchFrame)(&zframe, (*z2zE1353).hd);
    zz5listz8z5structz0zzTrieBranchFramez9 zrest;
    CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&zrest);
    COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&zrest, (*z2zE1353).tl);
    struct zTrieBuilder z2zE1357;
    CREATE(zTrieBuilder)(&z2zE1357);
    {
      struct zNodeRef z2zE1355;
      CREATE(zNodeRef)(&z2zE1355);
      COPY(zNodeRef)(&z2zE1355, zbuilder.zroot);
      bool z2zE1356;
      z2zE1356 = zbuilder.zcomplete;
      z2zE1357.zcomplete = z2zE1356;
      COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&((&z2zE1357)->zframes), zrest);
      COPY(zNodeRef)(&((&z2zE1357)->zroot), z2zE1355);
      KILL(zNodeRef)(&z2zE1355);
    }
    COPY(zTrieBranchFrame)(&((&z3zE1389)->ztup0), zframe);
    COPY(zTrieBuilder)(&((&z3zE1389)->ztup1), z2zE1357);
    KILL(zTrieBuilder)(&z2zE1357);
    KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&zrest);
    KILL(zTrieBranchFrame)(&zframe);
    goto finish_match_1833;
  }
case_1834: ;
finish_match_1833: ;
  COPY(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)((*(&z8zE535)), z3zE1389);
  KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z3zE1389);
  KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1353);
end_function_1836: ;
  goto end_function_4033;
end_block_exception_1837: ;
  goto end_function_4033;
end_function_4033: ;
}

void ztrie_builder_wrap_branch(struct zNodeRef *z8zE536, struct zTriePath zanchor, uint64_t zparent_depth, uint64_t zchild_depth, struct zNodeRef zchild)
{
  uint64_t zchild_start;
  {    zchild_start = (zparent_depth + UINT64_C(1));
  }
  bool z2zE1350;
  z2zE1350 = (!(zchild_start < zchild_depth));
  if (z2zE1350) {  COPY(zNodeRef)((*(&z8zE536)), zchild);  } else {
    uint64_t zgap;
    {    zgap = (zchild_depth - zchild_start);
    }
    struct zTriePath z2zE1352;
    {
      struct zTriePath z2zE1351;
      {
        z2zE1351 = zpath_drop(zanchor, zchild_start);
        if (have_exception) {  goto end_block_exception_1832;  }
      }
      {
        z2zE1352 = zpath_take(z2zE1351, zgap);
        if (have_exception) {  goto end_block_exception_1832;  }
      }
    }
    {
      zextension_child_ref((*(&z8zE536)), z2zE1352, zchild);
      if (have_exception) {  goto end_block_exception_1832;  }
    }
  }
end_function_1831: ;
  goto end_function_4032;
end_block_exception_1832: ;
  goto end_function_4032;
end_function_4032: ;
}

void ztrie_builder_close(struct zTrieBuilder *z8zE537, struct zTrieBuilder zbuilder, struct zTriePath zanchor, struct zoptionzIU64zK znext_common, uint64_t zfuel)
{
  bool z2zE1325;
  z2zE1325 = (zfuel == UINT64_C(0));
  if (z2zE1325) {
    struct zexception z2zE1326;
    CREATE(zexception)(&z2zE1326);
    zInvalidBlock(&z2zE1326, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE1326);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/updates.sail:234.8-234.44");
    KILL(zexception)(&z2zE1326);
    goto end_block_exception_1830;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1326);
  } else {
    zz5listz8z5structz0zzTrieBranchFramez9 z2zE1327;
    CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1327);
    COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1327, zbuilder.zframes);
    struct zTrieBuilder z3zE1368;
    CREATE(zTrieBuilder)(&z3zE1368);
    {
      if (!((z2zE1327 == NULL))) goto case_1828;
      COPY(zTrieBuilder)(&z3zE1368, zbuilder);
      goto finish_match_1812;
    }
  case_1828: ;
    {
      /* complete */
      struct zTrieBranchFrame ztop;
      CREATE(zTrieBranchFrame)(&ztop);
      COPY(zTrieBranchFrame)(&ztop, (*z2zE1327).hd);
      bool zshould_close;
      {
        bool z3zE1369;
        {
          if (znext_common.kind != Kind_zNonezIU64zK) goto case_1816;
          z3zE1369 = true;
          goto finish_match_1814;
        }
      case_1816: ;
        {
          /* complete */
          uint64_t zdepth;
          zdepth = znext_common.variants.zSomezIU64zK;
          uint64_t z2zE1349;
          z2zE1349 = ztop.zdepth;
          z3zE1369 = (zdepth < z2zE1349);
          goto finish_match_1814;
        }
      case_1815: ;
      finish_match_1814: ;
        zshould_close = z3zE1369;
      }
      bool z2zE1328;
      z2zE1328 = not(zshould_close);
      if (z2zE1328) {  COPY(zTrieBuilder)(&z3zE1368, zbuilder);  } else {
        struct ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9 z2zE1329;
        CREATE(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE1329);
        {
          ztrie_builder_pop(&z2zE1329, zbuilder);
          if (have_exception) {
            KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1327);
            KILL(zTrieBuilder)(&z3zE1368);
            KILL(zTrieBranchFrame)(&ztop);
            KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE1329);
            goto end_block_exception_1830;
          }
        }
        struct zTrieBuilder z3zE1373;
        CREATE(zTrieBuilder)(&z3zE1373);
        {
          struct zTrieBranchFrame zframe;
          CREATE(zTrieBranchFrame)(&zframe);
          COPY(zTrieBranchFrame)(&zframe, z2zE1329.ztup0);
          struct zTrieBuilder zpopped;
          CREATE(zTrieBuilder)(&zpopped);
          COPY(zTrieBuilder)(&zpopped, z2zE1329.ztup1);
          struct zNodeRef zchild;
          CREATE(zNodeRef)(&zchild);
          {
            uint64_t z2zE1347;
            z2zE1347 = zframe.zmask;
            zz5vecz8z5unionz0zzNodeRefz9 z2zE1348;
            CREATE(zz5vecz8z5unionz0zzNodeRefz9)(&z2zE1348);
            COPY(zz5vecz8z5unionz0zzNodeRefz9)(&z2zE1348, zframe.zchildren);
            {
              zbranch_child_ref(&zchild, z2zE1347, z2zE1348);
              if (have_exception) {
                KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1327);
                KILL(zTrieBuilder)(&z3zE1368);
                KILL(zTrieBranchFrame)(&ztop);
                KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE1329);
                KILL(zTrieBuilder)(&z3zE1373);
                KILL(zTrieBranchFrame)(&zframe);
                KILL(zTrieBuilder)(&zpopped);
                KILL(zNodeRef)(&zchild);
                KILL(zz5vecz8z5unionz0zzNodeRefz9)(&z2zE1348);
                goto end_block_exception_1830;
              }
            }
            KILL(zz5vecz8z5unionz0zzNodeRefz9)(&z2zE1348);
          }
          struct zTrieBuilder zwith_parent;
          CREATE(zTrieBuilder)(&zwith_parent);
          {
            zz5listz8z5structz0zzTrieBranchFramez9 z2zE1331;
            CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1331);
            COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1331, zpopped.zframes);
            struct zTrieBuilder z3zE1374;
            CREATE(zTrieBuilder)(&z3zE1374);
            {
              if ((z2zE1331 == NULL)) goto case_1824;
              struct zTrieBranchFrame zparent;
              CREATE(zTrieBranchFrame)(&zparent);
              COPY(zTrieBranchFrame)(&zparent, (*z2zE1331).hd);
              struct zTrieBuilder z3zE1380;
              CREATE(zTrieBuilder)(&z3zE1380);
              {
                if (znext_common.kind != Kind_zSomezIU64zK) goto case_1827;
                uint64_t zcommon;
                zcommon = znext_common.variants.zSomezIU64zK;
                bool z2zE1333;
                {
                  uint64_t z2zE1332;
                  z2zE1332 = zparent.zdepth;
                  z2zE1333 = (z2zE1332 < zcommon);
                }
                if (z2zE1333) {
                  struct zTrieBuilder zintermediate;
                  CREATE(zTrieBuilder)(&zintermediate);
                  ztrie_builder_push(&zintermediate, zpopped, zcommon);
                  struct zNodeRef z2zE1335;
                  CREATE(zNodeRef)(&z2zE1335);
                  {
                    uint64_t z2zE1334;
                    z2zE1334 = zframe.zdepth;
                    {
                      ztrie_builder_wrap_branch(&z2zE1335, zanchor, zcommon, z2zE1334, zchild);
                      if (have_exception) {
                        KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1327);
                        KILL(zTrieBuilder)(&z3zE1368);
                        KILL(zTrieBranchFrame)(&ztop);
                        KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE1329);
                        KILL(zTrieBuilder)(&z3zE1373);
                        KILL(zTrieBranchFrame)(&zframe);
                        KILL(zTrieBuilder)(&zpopped);
                        KILL(zNodeRef)(&zchild);
                        KILL(zTrieBuilder)(&zwith_parent);
                        KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1331);
                        KILL(zTrieBuilder)(&z3zE1374);
                        KILL(zTrieBranchFrame)(&zparent);
                        KILL(zTrieBuilder)(&z3zE1380);
                        KILL(zTrieBuilder)(&zintermediate);
                        KILL(zNodeRef)(&z2zE1335);
                        goto end_block_exception_1830;
                      }
                    }
                  }
                  {
                    ztrie_builder_attach(&z3zE1380, zintermediate, zanchor, z2zE1335);
                    if (have_exception) {
                      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1327);
                      KILL(zTrieBuilder)(&z3zE1368);
                      KILL(zTrieBranchFrame)(&ztop);
                      KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE1329);
                      KILL(zTrieBuilder)(&z3zE1373);
                      KILL(zTrieBranchFrame)(&zframe);
                      KILL(zTrieBuilder)(&zpopped);
                      KILL(zNodeRef)(&zchild);
                      KILL(zTrieBuilder)(&zwith_parent);
                      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1331);
                      KILL(zTrieBuilder)(&z3zE1374);
                      KILL(zTrieBranchFrame)(&zparent);
                      KILL(zTrieBuilder)(&z3zE1380);
                      KILL(zTrieBuilder)(&zintermediate);
                      KILL(zNodeRef)(&z2zE1335);
                      goto end_block_exception_1830;
                    }
                  }
                  KILL(zNodeRef)(&z2zE1335);
                  KILL(zTrieBuilder)(&zintermediate);
                } else {
                  struct zNodeRef z2zE1338;
                  CREATE(zNodeRef)(&z2zE1338);
                  {
                    uint64_t z2zE1336;
                    z2zE1336 = zparent.zdepth;
                    uint64_t z2zE1337;
                    z2zE1337 = zframe.zdepth;
                    {
                      ztrie_builder_wrap_branch(&z2zE1338, zanchor, z2zE1336, z2zE1337, zchild);
                      if (have_exception) {
                        KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1327);
                        KILL(zTrieBuilder)(&z3zE1368);
                        KILL(zTrieBranchFrame)(&ztop);
                        KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE1329);
                        KILL(zTrieBuilder)(&z3zE1373);
                        KILL(zTrieBranchFrame)(&zframe);
                        KILL(zTrieBuilder)(&zpopped);
                        KILL(zNodeRef)(&zchild);
                        KILL(zTrieBuilder)(&zwith_parent);
                        KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1331);
                        KILL(zTrieBuilder)(&z3zE1374);
                        KILL(zTrieBranchFrame)(&zparent);
                        KILL(zTrieBuilder)(&z3zE1380);
                        KILL(zNodeRef)(&z2zE1338);
                        goto end_block_exception_1830;
                      }
                    }
                  }
                  {
                    ztrie_builder_attach(&z3zE1380, zpopped, zanchor, z2zE1338);
                    if (have_exception) {
                      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1327);
                      KILL(zTrieBuilder)(&z3zE1368);
                      KILL(zTrieBranchFrame)(&ztop);
                      KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE1329);
                      KILL(zTrieBuilder)(&z3zE1373);
                      KILL(zTrieBranchFrame)(&zframe);
                      KILL(zTrieBuilder)(&zpopped);
                      KILL(zNodeRef)(&zchild);
                      KILL(zTrieBuilder)(&zwith_parent);
                      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1331);
                      KILL(zTrieBuilder)(&z3zE1374);
                      KILL(zTrieBranchFrame)(&zparent);
                      KILL(zTrieBuilder)(&z3zE1380);
                      KILL(zNodeRef)(&z2zE1338);
                      goto end_block_exception_1830;
                    }
                  }
                  KILL(zNodeRef)(&z2zE1338);
                }
                goto finish_match_1825;
              }
            case_1827: ;
              {
                /* complete */
                struct zNodeRef z2zE1341;
                CREATE(zNodeRef)(&z2zE1341);
                {
                  uint64_t z2zE1339;
                  z2zE1339 = zparent.zdepth;
                  uint64_t z2zE1340;
                  z2zE1340 = zframe.zdepth;
                  {
                    ztrie_builder_wrap_branch(&z2zE1341, zanchor, z2zE1339, z2zE1340, zchild);
                    if (have_exception) {
                      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1327);
                      KILL(zTrieBuilder)(&z3zE1368);
                      KILL(zTrieBranchFrame)(&ztop);
                      KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE1329);
                      KILL(zTrieBuilder)(&z3zE1373);
                      KILL(zTrieBranchFrame)(&zframe);
                      KILL(zTrieBuilder)(&zpopped);
                      KILL(zNodeRef)(&zchild);
                      KILL(zTrieBuilder)(&zwith_parent);
                      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1331);
                      KILL(zTrieBuilder)(&z3zE1374);
                      KILL(zTrieBranchFrame)(&zparent);
                      KILL(zTrieBuilder)(&z3zE1380);
                      KILL(zNodeRef)(&z2zE1341);
                      goto end_block_exception_1830;
                    }
                  }
                }
                {
                  ztrie_builder_attach(&z3zE1380, zpopped, zanchor, z2zE1341);
                  if (have_exception) {
                    KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1327);
                    KILL(zTrieBuilder)(&z3zE1368);
                    KILL(zTrieBranchFrame)(&ztop);
                    KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE1329);
                    KILL(zTrieBuilder)(&z3zE1373);
                    KILL(zTrieBranchFrame)(&zframe);
                    KILL(zTrieBuilder)(&zpopped);
                    KILL(zNodeRef)(&zchild);
                    KILL(zTrieBuilder)(&zwith_parent);
                    KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1331);
                    KILL(zTrieBuilder)(&z3zE1374);
                    KILL(zTrieBranchFrame)(&zparent);
                    KILL(zTrieBuilder)(&z3zE1380);
                    KILL(zNodeRef)(&z2zE1341);
                    goto end_block_exception_1830;
                  }
                }
                KILL(zNodeRef)(&z2zE1341);
                goto finish_match_1825;
              }
            case_1826: ;
            finish_match_1825: ;
              COPY(zTrieBuilder)(&z3zE1374, z3zE1380);
              KILL(zTrieBuilder)(&z3zE1380);
              KILL(zTrieBranchFrame)(&zparent);
              goto finish_match_1819;
            }
          case_1824: ;
            {
              /* complete */
              struct zTrieBuilder z3zE1375;
              CREATE(zTrieBuilder)(&z3zE1375);
              {
                if (znext_common.kind != Kind_zSomezIU64zK) goto case_1823;
                uint64_t z3zE1386;
                z3zE1386 = znext_common.variants.zSomezIU64zK;
                struct zTrieBuilder z3zE1387;
                CREATE(zTrieBuilder)(&z3zE1387);
                ztrie_builder_push(&z3zE1387, zpopped, z3zE1386);
                struct zNodeRef z2zE1343;
                CREATE(zNodeRef)(&z2zE1343);
                {
                  uint64_t z2zE1342;
                  z2zE1342 = zframe.zdepth;
                  {
                    ztrie_builder_wrap_branch(&z2zE1343, zanchor, z3zE1386, z2zE1342, zchild);
                    if (have_exception) {
                      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1327);
                      KILL(zTrieBuilder)(&z3zE1368);
                      KILL(zTrieBranchFrame)(&ztop);
                      KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE1329);
                      KILL(zTrieBuilder)(&z3zE1373);
                      KILL(zTrieBranchFrame)(&zframe);
                      KILL(zTrieBuilder)(&zpopped);
                      KILL(zNodeRef)(&zchild);
                      KILL(zTrieBuilder)(&zwith_parent);
                      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1331);
                      KILL(zTrieBuilder)(&z3zE1374);
                      KILL(zTrieBuilder)(&z3zE1375);
                      KILL(zTrieBuilder)(&z3zE1387);
                      KILL(zNodeRef)(&z2zE1343);
                      goto end_block_exception_1830;
                    }
                  }
                }
                {
                  ztrie_builder_attach(&z3zE1375, z3zE1387, zanchor, z2zE1343);
                  if (have_exception) {
                    KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1327);
                    KILL(zTrieBuilder)(&z3zE1368);
                    KILL(zTrieBranchFrame)(&ztop);
                    KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE1329);
                    KILL(zTrieBuilder)(&z3zE1373);
                    KILL(zTrieBranchFrame)(&zframe);
                    KILL(zTrieBuilder)(&zpopped);
                    KILL(zNodeRef)(&zchild);
                    KILL(zTrieBuilder)(&zwith_parent);
                    KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1331);
                    KILL(zTrieBuilder)(&z3zE1374);
                    KILL(zTrieBuilder)(&z3zE1375);
                    KILL(zTrieBuilder)(&z3zE1387);
                    KILL(zNodeRef)(&z2zE1343);
                    goto end_block_exception_1830;
                  }
                }
                KILL(zNodeRef)(&z2zE1343);
                KILL(zTrieBuilder)(&z3zE1387);
                goto finish_match_1821;
              }
            case_1823: ;
              {
                /* complete */
                uint64_t z3zE1388;
                z3zE1388 = zframe.zdepth;
                struct zNodeRef zroot;
                CREATE(zNodeRef)(&zroot);
                {
                  bool z2zE1345;
                  z2zE1345 = (z3zE1388 == UINT64_C(0));
                  if (z2zE1345) {  COPY(zNodeRef)(&zroot, zchild);  } else {
                    struct zTriePath z2zE1346;
                    {
                      z2zE1346 = zpath_take(zanchor, z3zE1388);
                      if (have_exception) {
                        KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1327);
                        KILL(zTrieBuilder)(&z3zE1368);
                        KILL(zTrieBranchFrame)(&ztop);
                        KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE1329);
                        KILL(zTrieBuilder)(&z3zE1373);
                        KILL(zTrieBranchFrame)(&zframe);
                        KILL(zTrieBuilder)(&zpopped);
                        KILL(zNodeRef)(&zchild);
                        KILL(zTrieBuilder)(&zwith_parent);
                        KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1331);
                        KILL(zTrieBuilder)(&z3zE1374);
                        KILL(zTrieBuilder)(&z3zE1375);
                        KILL(zNodeRef)(&zroot);
                        goto end_block_exception_1830;
                      }
                    }
                    {
                      zextension_child_ref(&zroot, z2zE1346, zchild);
                      if (have_exception) {
                        KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1327);
                        KILL(zTrieBuilder)(&z3zE1368);
                        KILL(zTrieBranchFrame)(&ztop);
                        KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE1329);
                        KILL(zTrieBuilder)(&z3zE1373);
                        KILL(zTrieBranchFrame)(&zframe);
                        KILL(zTrieBuilder)(&zpopped);
                        KILL(zNodeRef)(&zchild);
                        KILL(zTrieBuilder)(&zwith_parent);
                        KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1331);
                        KILL(zTrieBuilder)(&z3zE1374);
                        KILL(zTrieBuilder)(&z3zE1375);
                        KILL(zNodeRef)(&zroot);
                        goto end_block_exception_1830;
                      }
                    }
                  }
                }
                zz5listz8z5structz0zzTrieBranchFramez9 z2zE1344;
                CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1344);
                COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1344, zpopped.zframes);
                z3zE1375.zcomplete = true;
                COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&((&z3zE1375)->zframes), z2zE1344);
                COPY(zNodeRef)(&((&z3zE1375)->zroot), zroot);
                KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1344);
                KILL(zNodeRef)(&zroot);
                goto finish_match_1821;
              }
            case_1822: ;
            finish_match_1821: ;
              COPY(zTrieBuilder)(&z3zE1374, z3zE1375);
              KILL(zTrieBuilder)(&z3zE1375);
              goto finish_match_1819;
            }
          case_1820: ;
          finish_match_1819: ;
            COPY(zTrieBuilder)(&zwith_parent, z3zE1374);
            KILL(zTrieBuilder)(&z3zE1374);
            KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1331);
          }
          uint64_t z2zE1330;
          {    z2zE1330 = (zfuel - UINT64_C(1));
          }
          {
            ztrie_builder_close(&z3zE1373, zwith_parent, zanchor, znext_common, z2zE1330);
            if (have_exception) {
              KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1327);
              KILL(zTrieBuilder)(&z3zE1368);
              KILL(zTrieBranchFrame)(&ztop);
              KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE1329);
              KILL(zTrieBuilder)(&z3zE1373);
              KILL(zTrieBranchFrame)(&zframe);
              KILL(zTrieBuilder)(&zpopped);
              KILL(zNodeRef)(&zchild);
              KILL(zTrieBuilder)(&zwith_parent);
              goto end_block_exception_1830;
            }
          }
          KILL(zTrieBuilder)(&zwith_parent);
          KILL(zNodeRef)(&zchild);
          KILL(zTrieBuilder)(&zpopped);
          KILL(zTrieBranchFrame)(&zframe);
          goto finish_match_1817;
        }
      case_1818: ;
        sail_match_failure("trie_builder_close");
      finish_match_1817: ;
        COPY(zTrieBuilder)(&z3zE1368, z3zE1373);
        KILL(zTrieBuilder)(&z3zE1373);
        KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE1329);
      }
      KILL(zTrieBranchFrame)(&ztop);
      goto finish_match_1812;
    }
  case_1813: ;
  finish_match_1812: ;
    COPY(zTrieBuilder)((*(&z8zE537)), z3zE1368);
    KILL(zTrieBuilder)(&z3zE1368);
    KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1327);
  }
end_function_1829: ;
  goto end_function_4031;
end_block_exception_1830: ;
  goto end_function_4031;
end_function_4031: ;
}

void ztrie_item_next_common(struct zoptionzIU64zK *z8zE538, struct zTrieItem zitem, struct zoptionzIRTriePathzK znext_key)
{
  struct zoptionzIU64zK z3zE1362;
  CREATE(zoptionzIU64zK)(&z3zE1362);
  {
    if (znext_key.kind != Kind_zNonezIRTriePathzK) goto case_1809;
    zNonezIU64zK(&z3zE1362, UNIT);
    goto finish_match_1807;
  }
case_1809: ;
  {
    /* complete */
    struct zTriePath znext;
    znext = znext_key.variants.zSomezIRTriePathzK;
    uint64_t zcommon;
    {
      struct zTriePath z2zE1324;
      z2zE1324 = zitem.zpath;
      {
        zcommon = zcommon_prefix_from(z2zE1324, znext, UINT64_C(0));
        if (have_exception) {
          KILL(zoptionzIU64zK)(&z3zE1362);
          goto end_block_exception_1811;
        }
      }
    }
    bool z2zE1321;
    {
      bool z2zE1320;
      {
        bool z2zE1315;
        {
          struct zTriePath z2zE1314;
          z2zE1314 = zitem.zpath;
          z2zE1315 = zpath_lt(z2zE1314, znext);
        }
        z2zE1320 = not(z2zE1315);
      }
      bool z3zE1364;
      if (z2zE1320) {  z3zE1364 = true;  } else {
        bool z2zE1319;
        {
          uint64_t z2zE1317;
          {
            struct zTriePath z2zE1316;
            z2zE1316 = zitem.zpath;
            z2zE1317 = zpath_len(z2zE1316);
          }
          z2zE1319 = (!(zcommon < z2zE1317));
        }
        bool z3zE1363;
        if (z2zE1319) {  z3zE1363 = true;  } else {
          uint64_t z2zE1318;
          z2zE1318 = zpath_len(znext);
          z3zE1363 = (!(zcommon < z2zE1318));
        }
        z3zE1364 = z3zE1363;
      }
      z2zE1321 = z3zE1364;
    }
    unit z3zE1365;
    if (z2zE1321) {
      struct zexception z2zE1322;
      CREATE(zexception)(&z2zE1322);
      zInvalidBlock(&z2zE1322, zWitnessDeficient);
      COPY(zexception)(current_exception, z2zE1322);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/lib/mpt/updates.sail:305.16-305.52");
      KILL(zoptionzIU64zK)(&z3zE1362);
      KILL(zexception)(&z2zE1322);
      goto end_block_exception_1811;
      /* unreachable after throw */
      KILL(zexception)(&z2zE1322);
    } else {  z3zE1365 = UNIT;  }
    uint64_t z2zE1323;
    {
      z2zE1323 = zto_trie_depth(zcommon);
      if (have_exception) {
        KILL(zoptionzIU64zK)(&z3zE1362);
        goto end_block_exception_1811;
      }
    }
    zSomezIU64zK(&z3zE1362, z2zE1323);
    goto finish_match_1807;
  }
case_1808: ;
finish_match_1807: ;
  COPY(zoptionzIU64zK)((*(&z8zE538)), z3zE1362);
  KILL(zoptionzIU64zK)(&z3zE1362);
end_function_1810: ;
  goto end_function_4030;
end_block_exception_1811: ;
  goto end_function_4030;
end_function_4030: ;
}

void ztrie_insert_item(struct zTrieBuilder *z8zE539, struct zTrieBuilder zbuilder, struct zTrieItem zitem, struct zoptionzIRTriePathzK znext_key)
{
  bool z2zE1298;
  z2zE1298 = zbuilder.zcomplete;
  unit z3zE1346;
  if (z2zE1298) {
    struct zexception z2zE1299;
    CREATE(zexception)(&z2zE1299);
    zInvalidBlock(&z2zE1299, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE1299);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/updates.sail:315.8-315.44");
    KILL(zexception)(&z2zE1299);
    goto end_block_exception_1806;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1299);
  } else {  z3zE1346 = UNIT;  }
  struct zoptionzIU64zK znext_common;
  CREATE(zoptionzIU64zK)(&znext_common);
  {
    ztrie_item_next_common(&znext_common, zitem, znext_key);
    if (have_exception) {
      KILL(zoptionzIU64zK)(&znext_common);
      goto end_block_exception_1806;
    }
  }
  bool zopen_child;
  {
    bool z3zE1347;
    {
      if (znext_common.kind != Kind_zNonezIU64zK) goto case_1798;
      z3zE1347 = false;
      goto finish_match_1793;
    }
  case_1798: ;
    {
      /* complete */
      uint64_t zcommon;
      zcommon = znext_common.variants.zSomezIU64zK;
      zz5listz8z5structz0zzTrieBranchFramez9 z2zE1312;
      CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1312);
      COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1312, zbuilder.zframes);
      bool z3zE1348;
      {
        if (!((z2zE1312 == NULL))) goto case_1797;
        z3zE1348 = true;
        goto finish_match_1795;
      }
    case_1797: ;
      {
        /* complete */
        struct zTrieBranchFrame ztop;
        CREATE(zTrieBranchFrame)(&ztop);
        COPY(zTrieBranchFrame)(&ztop, (*z2zE1312).hd);
        uint64_t z2zE1313;
        z2zE1313 = ztop.zdepth;
        z3zE1348 = (z2zE1313 < zcommon);
        KILL(zTrieBranchFrame)(&ztop);
        goto finish_match_1795;
      }
    case_1796: ;
    finish_match_1795: ;
      z3zE1347 = z3zE1348;
      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1312);
      goto finish_match_1793;
    }
  case_1794: ;
  finish_match_1793: ;
    zopen_child = z3zE1347;
  }
  if (zopen_child) {
    struct zTrieBuilder z3zE1357;
    CREATE(zTrieBuilder)(&z3zE1357);
    {
      if (znext_common.kind != Kind_zSomezIU64zK) goto case_1804;
      uint64_t z3zE1360;
      z3zE1360 = znext_common.variants.zSomezIU64zK;
      struct zTrieBuilder zopened;
      CREATE(zTrieBuilder)(&zopened);
      ztrie_builder_push(&zopened, zbuilder, z3zE1360);
      struct zTriePath z2zE1301;
      z2zE1301 = zitem.zpath;
      struct zNodeRef z2zE1302;
      CREATE(zNodeRef)(&z2zE1302);
      {
        uint64_t z2zE1300;
        {    z2zE1300 = (z3zE1360 + UINT64_C(1));
        }
        {
          zitem_ref(&z2zE1302, zitem, z2zE1300);
          if (have_exception) {
            KILL(zoptionzIU64zK)(&znext_common);
            KILL(zTrieBuilder)(&z3zE1357);
            KILL(zTrieBuilder)(&zopened);
            KILL(zNodeRef)(&z2zE1302);
            goto end_block_exception_1806;
          }
        }
      }
      {
        ztrie_builder_attach(&z3zE1357, zopened, z2zE1301, z2zE1302);
        if (have_exception) {
          KILL(zoptionzIU64zK)(&znext_common);
          KILL(zTrieBuilder)(&z3zE1357);
          KILL(zTrieBuilder)(&zopened);
          KILL(zNodeRef)(&z2zE1302);
          goto end_block_exception_1806;
        }
      }
      KILL(zNodeRef)(&z2zE1302);
      KILL(zTrieBuilder)(&zopened);
      goto finish_match_1802;
    }
  case_1804: ;
    {
      /* complete */
      struct zexception z2zE1303;
      CREATE(zexception)(&z2zE1303);
      zInvalidBlock(&z2zE1303, zWitnessDeficient);
      COPY(zexception)(current_exception, z2zE1303);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/lib/mpt/updates.sail:331.22-331.58");
      KILL(zoptionzIU64zK)(&znext_common);
      KILL(zTrieBuilder)(&z3zE1357);
      KILL(zexception)(&z2zE1303);
      goto end_block_exception_1806;
      /* unreachable after throw */
      KILL(zexception)(&z2zE1303);
      goto finish_match_1802;
    }
  case_1803: ;
  finish_match_1802: ;
    COPY(zTrieBuilder)((*(&z8zE539)), z3zE1357);
    KILL(zTrieBuilder)(&z3zE1357);
  } else {
    zz5listz8z5structz0zzTrieBranchFramez9 z2zE1304;
    CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1304);
    COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1304, zbuilder.zframes);
    struct zTrieBuilder z3zE1353;
    CREATE(zTrieBuilder)(&z3zE1353);
    {
      if (!((z2zE1304 == NULL))) goto case_1801;
      zz5listz8z5structz0zzTrieBranchFramez9 z2zE1305;
      CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1305);
      COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1305, zbuilder.zframes);
      struct zNodeRef z2zE1306;
      CREATE(zNodeRef)(&z2zE1306);
      {
        zitem_ref(&z2zE1306, zitem, UINT64_C(0));
        if (have_exception) {
          KILL(zoptionzIU64zK)(&znext_common);
          KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1304);
          KILL(zTrieBuilder)(&z3zE1353);
          KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1305);
          KILL(zNodeRef)(&z2zE1306);
          goto end_block_exception_1806;
        }
      }
      z3zE1353.zcomplete = true;
      COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&((&z3zE1353)->zframes), z2zE1305);
      COPY(zNodeRef)(&((&z3zE1353)->zroot), z2zE1306);
      KILL(zNodeRef)(&z2zE1306);
      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1305);
      goto finish_match_1799;
    }
  case_1801: ;
    {
      /* complete */
      struct zTrieBranchFrame z3zE1361;
      CREATE(zTrieBranchFrame)(&z3zE1361);
      COPY(zTrieBranchFrame)(&z3zE1361, (*z2zE1304).hd);
      struct zTrieBuilder zattached;
      CREATE(zTrieBuilder)(&zattached);
      {
        struct zTriePath z2zE1310;
        z2zE1310 = zitem.zpath;
        struct zNodeRef z2zE1311;
        CREATE(zNodeRef)(&z2zE1311);
        {
          uint64_t z2zE1309;
          {
            uint64_t z2zE1308;
            z2zE1308 = z3zE1361.zdepth;
            {    z2zE1309 = (z2zE1308 + UINT64_C(1));
            }
          }
          {
            zitem_ref(&z2zE1311, zitem, z2zE1309);
            if (have_exception) {
              KILL(zoptionzIU64zK)(&znext_common);
              KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1304);
              KILL(zTrieBuilder)(&z3zE1353);
              KILL(zTrieBranchFrame)(&z3zE1361);
              KILL(zTrieBuilder)(&zattached);
              KILL(zNodeRef)(&z2zE1311);
              goto end_block_exception_1806;
            }
          }
        }
        {
          ztrie_builder_attach(&zattached, zbuilder, z2zE1310, z2zE1311);
          if (have_exception) {
            KILL(zoptionzIU64zK)(&znext_common);
            KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1304);
            KILL(zTrieBuilder)(&z3zE1353);
            KILL(zTrieBranchFrame)(&z3zE1361);
            KILL(zTrieBuilder)(&zattached);
            KILL(zNodeRef)(&z2zE1311);
            goto end_block_exception_1806;
          }
        }
        KILL(zNodeRef)(&z2zE1311);
      }
      struct zTriePath z2zE1307;
      z2zE1307 = zitem.zpath;
      {
        ztrie_builder_close(&z3zE1353, zattached, z2zE1307, znext_common, UINT64_C(64));
        if (have_exception) {
          KILL(zoptionzIU64zK)(&znext_common);
          KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1304);
          KILL(zTrieBuilder)(&z3zE1353);
          KILL(zTrieBranchFrame)(&z3zE1361);
          KILL(zTrieBuilder)(&zattached);
          goto end_block_exception_1806;
        }
      }
      KILL(zTrieBuilder)(&zattached);
      KILL(zTrieBranchFrame)(&z3zE1361);
      goto finish_match_1799;
    }
  case_1800: ;
  finish_match_1799: ;
    COPY(zTrieBuilder)((*(&z8zE539)), z3zE1353);
    KILL(zTrieBuilder)(&z3zE1353);
    KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1304);
  }
  KILL(zoptionzIU64zK)(&znext_common);
end_function_1805: ;
  goto end_function_4029;
end_block_exception_1806: ;
  goto end_function_4029;
end_function_4029: ;
}

void ztrie_sink_empty(struct zTrieItemSink *z8zE540, unit z3zE1344)
{
  struct zTrieBuilder z2zE1296;
  CREATE(zTrieBuilder)(&z2zE1296);
  ztrie_builder_empty(&z2zE1296, UNIT);
  struct zoptionzIRTrieItemzK z2zE1297;
  CREATE(zoptionzIRTrieItemzK)(&z2zE1297);
  zNonezIRTrieItemzK(&z2zE1297, UNIT);
  struct zTrieItemSink z3zE1345;
  CREATE(zTrieItemSink)(&z3zE1345);
  COPY(zTrieBuilder)(&((&z3zE1345)->zbuilder), z2zE1296);
  COPY(zoptionzIRTrieItemzK)(&((&z3zE1345)->zpending), z2zE1297);
  COPY(zTrieItemSink)((*(&z8zE540)), z3zE1345);
  KILL(zTrieItemSink)(&z3zE1345);
  KILL(zoptionzIRTrieItemzK)(&z2zE1297);
  KILL(zTrieBuilder)(&z2zE1296);
end_function_1791: ;
  goto end_function_4028;
end_block_exception_1792: ;
  goto end_function_4028;
end_function_4028: ;
}

void ztrie_sink_emit(struct zTrieItemSink *z8zE541, struct zTrieItemSink zsink, struct zTrieItem zitem)
{
  struct zoptionzIRTrieItemzK z2zE1288;
  CREATE(zoptionzIRTrieItemzK)(&z2zE1288);
  COPY(zoptionzIRTrieItemzK)(&z2zE1288, zsink.zpending);
  struct zTrieItemSink z3zE1339;
  CREATE(zTrieItemSink)(&z3zE1339);
  {
    if (z2zE1288.kind != Kind_zNonezIRTrieItemzK) goto case_1788;
    struct zTrieBuilder z2zE1289;
    CREATE(zTrieBuilder)(&z2zE1289);
    COPY(zTrieBuilder)(&z2zE1289, zsink.zbuilder);
    struct zoptionzIRTrieItemzK z2zE1290;
    CREATE(zoptionzIRTrieItemzK)(&z2zE1290);
    zSomezIRTrieItemzK(&z2zE1290, zitem);
    COPY(zTrieBuilder)(&((&z3zE1339)->zbuilder), z2zE1289);
    COPY(zoptionzIRTrieItemzK)(&((&z3zE1339)->zpending), z2zE1290);
    KILL(zoptionzIRTrieItemzK)(&z2zE1290);
    KILL(zTrieBuilder)(&z2zE1289);
    goto finish_match_1786;
  }
case_1788: ;
  {
    /* complete */
    struct zTrieItem zprevious;
    CREATE(zTrieItem)(&zprevious);
    COPY(zTrieItem)(&zprevious, z2zE1288.variants.zSomezIRTrieItemzK);
    struct zTrieBuilder z2zE1294;
    CREATE(zTrieBuilder)(&z2zE1294);
    {
      struct zTrieBuilder z2zE1292;
      CREATE(zTrieBuilder)(&z2zE1292);
      COPY(zTrieBuilder)(&z2zE1292, zsink.zbuilder);
      struct zoptionzIRTriePathzK z2zE1293;
      CREATE(zoptionzIRTriePathzK)(&z2zE1293);
      {
        struct zTriePath z2zE1291;
        z2zE1291 = zitem.zpath;
        zSomezIRTriePathzK(&z2zE1293, z2zE1291);
      }
      {
        ztrie_insert_item(&z2zE1294, z2zE1292, zprevious, z2zE1293);
        if (have_exception) {
          KILL(zoptionzIRTrieItemzK)(&z2zE1288);
          KILL(zTrieItemSink)(&z3zE1339);
          KILL(zTrieItem)(&zprevious);
          KILL(zTrieBuilder)(&z2zE1294);
          KILL(zTrieBuilder)(&z2zE1292);
          KILL(zoptionzIRTriePathzK)(&z2zE1293);
          goto end_block_exception_1790;
        }
      }
      KILL(zoptionzIRTriePathzK)(&z2zE1293);
      KILL(zTrieBuilder)(&z2zE1292);
    }
    struct zoptionzIRTrieItemzK z2zE1295;
    CREATE(zoptionzIRTrieItemzK)(&z2zE1295);
    zSomezIRTrieItemzK(&z2zE1295, zitem);
    COPY(zTrieBuilder)(&((&z3zE1339)->zbuilder), z2zE1294);
    COPY(zoptionzIRTrieItemzK)(&((&z3zE1339)->zpending), z2zE1295);
    KILL(zoptionzIRTrieItemzK)(&z2zE1295);
    KILL(zTrieBuilder)(&z2zE1294);
    KILL(zTrieItem)(&zprevious);
    goto finish_match_1786;
  }
case_1787: ;
finish_match_1786: ;
  COPY(zTrieItemSink)((*(&z8zE541)), z3zE1339);
  KILL(zTrieItemSink)(&z3zE1339);
  KILL(zoptionzIRTrieItemzK)(&z2zE1288);
end_function_1789: ;
  goto end_function_4027;
end_block_exception_1790: ;
  goto end_function_4027;
end_function_4027: ;
}

void ztrie_sink_finish(struct zTrieItemSink *z8zE542, struct zTrieItemSink zsink)
{
  struct zoptionzIRTrieItemzK z2zE1283;
  CREATE(zoptionzIRTrieItemzK)(&z2zE1283);
  COPY(zoptionzIRTrieItemzK)(&z2zE1283, zsink.zpending);
  struct zTrieItemSink z3zE1335;
  CREATE(zTrieItemSink)(&z3zE1335);
  {
    if (z2zE1283.kind != Kind_zNonezIRTrieItemzK) goto case_1783;
    COPY(zTrieItemSink)(&z3zE1335, zsink);
    goto finish_match_1781;
  }
case_1783: ;
  {
    /* complete */
    struct zTrieItem zitem;
    CREATE(zTrieItem)(&zitem);
    COPY(zTrieItem)(&zitem, z2zE1283.variants.zSomezIRTrieItemzK);
    struct zTrieBuilder z2zE1286;
    CREATE(zTrieBuilder)(&z2zE1286);
    {
      struct zTrieBuilder z2zE1284;
      CREATE(zTrieBuilder)(&z2zE1284);
      COPY(zTrieBuilder)(&z2zE1284, zsink.zbuilder);
      struct zoptionzIRTriePathzK z2zE1285;
      CREATE(zoptionzIRTriePathzK)(&z2zE1285);
      zNonezIRTriePathzK(&z2zE1285, UNIT);
      {
        ztrie_insert_item(&z2zE1286, z2zE1284, zitem, z2zE1285);
        if (have_exception) {
          KILL(zoptionzIRTrieItemzK)(&z2zE1283);
          KILL(zTrieItemSink)(&z3zE1335);
          KILL(zTrieItem)(&zitem);
          KILL(zTrieBuilder)(&z2zE1286);
          KILL(zTrieBuilder)(&z2zE1284);
          KILL(zoptionzIRTriePathzK)(&z2zE1285);
          goto end_block_exception_1785;
        }
      }
      KILL(zoptionzIRTriePathzK)(&z2zE1285);
      KILL(zTrieBuilder)(&z2zE1284);
    }
    struct zoptionzIRTrieItemzK z2zE1287;
    CREATE(zoptionzIRTrieItemzK)(&z2zE1287);
    zNonezIRTrieItemzK(&z2zE1287, UNIT);
    COPY(zTrieBuilder)(&((&z3zE1335)->zbuilder), z2zE1286);
    COPY(zoptionzIRTrieItemzK)(&((&z3zE1335)->zpending), z2zE1287);
    KILL(zoptionzIRTrieItemzK)(&z2zE1287);
    KILL(zTrieBuilder)(&z2zE1286);
    KILL(zTrieItem)(&zitem);
    goto finish_match_1781;
  }
case_1782: ;
finish_match_1781: ;
  COPY(zTrieItemSink)((*(&z8zE542)), z3zE1335);
  KILL(zTrieItemSink)(&z3zE1335);
  KILL(zoptionzIRTrieItemzK)(&z2zE1283);
end_function_1784: ;
  goto end_function_4026;
end_block_exception_1785: ;
  goto end_function_4026;
end_function_4026: ;
}

sail_fixed_bytes_32 ztrie_builder_root(struct zTrieBuilder zbuilder)
{
  sail_fixed_bytes_32 z8zE543;
  zz5listz8z5structz0zzTrieBranchFramez9 z2zE1279;
  CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1279);
  COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1279, zbuilder.zframes);
  sail_fixed_bytes_32 z3zE1332;
  {
    if ((z2zE1279 == NULL)) goto case_1778;
    struct zexception z2zE1280;
    CREATE(zexception)(&z2zE1280);
    zInvalidBlock(&z2zE1280, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE1280);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/updates.sail:376.18-376.54");
    KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1279);
    KILL(zexception)(&z2zE1280);
    goto end_block_exception_1780;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1280);
    goto finish_match_1776;
  }
case_1778: ;
  {
    /* complete */
    bool z2zE1281;
    z2zE1281 = zbuilder.zcomplete;
    if (z2zE1281) {
      struct zNodeRef z2zE1282;
      CREATE(zNodeRef)(&z2zE1282);
      COPY(zNodeRef)(&z2zE1282, zbuilder.zroot);
      z3zE1332 = ztrie_ref_to_root(z2zE1282);
      KILL(zNodeRef)(&z2zE1282);
    } else {  z3zE1332 = zEMPTY_TRIE_ROOT;  }
    goto finish_match_1776;
  }
case_1777: ;
finish_match_1776: ;
  z8zE543 = z3zE1332;
  KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1279);
end_function_1779: ;
  return z8zE543;
end_block_exception_1780: ;

  return fixed_bytes_32_zero();
}

sail_fixed_bytes_32 ztrie_sink_root(struct zTrieItemSink zsink)
{
  sail_fixed_bytes_32 z8zE544;
  struct zoptionzIRTrieItemzK z2zE1276;
  CREATE(zoptionzIRTrieItemzK)(&z2zE1276);
  COPY(zoptionzIRTrieItemzK)(&z2zE1276, zsink.zpending);
  sail_fixed_bytes_32 z3zE1329;
  {
    if (z2zE1276.kind != Kind_zSomezIRTrieItemzK) goto case_1773;
    struct zexception z2zE1277;
    CREATE(zexception)(&z2zE1277);
    zInvalidBlock(&z2zE1277, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE1277);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/updates.sail:387.19-387.55");
    KILL(zoptionzIRTrieItemzK)(&z2zE1276);
    KILL(zexception)(&z2zE1277);
    goto end_block_exception_1775;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1277);
    goto finish_match_1771;
  }
case_1773: ;
  {
    /* complete */
    struct zTrieBuilder z2zE1278;
    CREATE(zTrieBuilder)(&z2zE1278);
    COPY(zTrieBuilder)(&z2zE1278, zsink.zbuilder);
    {
      z3zE1329 = ztrie_builder_root(z2zE1278);
      if (have_exception) {
        KILL(zoptionzIRTrieItemzK)(&z2zE1276);
        KILL(zTrieBuilder)(&z2zE1278);
        goto end_block_exception_1775;
      }
    }
    KILL(zTrieBuilder)(&z2zE1278);
    goto finish_match_1771;
  }
case_1772: ;
finish_match_1771: ;
  z8zE544 = z3zE1329;
  KILL(zoptionzIRTrieItemzK)(&z2zE1276);
end_function_1774: ;
  return z8zE544;
end_block_exception_1775: ;

  return fixed_bytes_32_zero();
}

