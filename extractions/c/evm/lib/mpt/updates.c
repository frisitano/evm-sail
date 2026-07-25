/* Generated from sail/lib/mpt/updates.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
void zcached_account_trie_update_next(struct zoptionzIRTrieUpdatezK *z8zE508, unit z3zE1474)
{
  zNonezIRTrieUpdatezK((*(&z8zE508)), UNIT);
end_function_1918: ;
  goto end_function_4032;
end_block_exception_1919: ;
  goto end_function_4032;
end_function_4032: ;
}

struct zByteSliceFields zencode_state_account(struct zAccountInfo zinfo, sail_fixed_bytes_32 zstorage_root)
{
  struct zByteSliceFields z8zE509;
  uint64_t znonce_length;
  {
    uint64_t z2zE1478;
    z2zE1478 = zinfo.znonce;
    znonce_length = zrlp_uint_word_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE1478);
  }
  uint64_t zbalance_length;
  {
    sail_u256 z2zE1477;
    z2zE1477 = zinfo.zbalance;
    zbalance_length = zrlp_uint_word_sizze(z2zE1477);
  }
  uint64_t zcontent_len;
  {
    uint64_t z2zE1476;
    {
      uint64_t z2zE1475;
      {    z2zE1475 = (znonce_length + zbalance_length);
      }
      {    z2zE1476 = (z2zE1475 + UINT64_C(33));
      }
    }
    {    zcontent_len = (z2zE1476 + UINT64_C(33));
    }
  }
  uint64_t zstart;
  zstart = zscratch_begin(UNIT);
  unit z3zE1473;
  z3zE1473 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zcontent_len);
  uint64_t z2zE1470;
  z2zE1470 = zinfo.znonce;
  unit z3zE1472;
  z3zE1472 = zrlp_write_uint_wordzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE1470);
  sail_u256 z2zE1471;
  z2zE1471 = zinfo.zbalance;
  unit z3zE1471;
  z3zE1471 = zrlp_write_uint_word(z2zE1471);
  sail_u256 z2zE1472;
  z2zE1472 = zhash_to_word(zstorage_root);
  unit z3zE1470;
  z3zE1470 = zrlp_write_word(z2zE1472);
  sail_u256 z2zE1474;
  {
    sail_fixed_bytes_32 z2zE1473;
    z2zE1473 = zinfo.zcode_hash;
    z2zE1474 = zhash_to_word(z2zE1473);
  }
  unit z3zE1469;
  z3zE1469 = zrlp_write_word(z2zE1474);
  z8zE509 = zrlp_finish(zstart);
end_function_1916: ;
  return z8zE509;
end_block_exception_1917: ;
  struct zByteSliceFields z8zE997 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE997;
}

void zaccount_update(struct zTrieUpdate *z8zE510, struct zAcctEntry zentry, sail_fixed_bytes_32 zstorage_root)
{
  struct zAccount zcurrent;
  {
    struct zAcctValue z2zE1469;
    z2zE1469 = zentry.zvalue;
    zcurrent = z2zE1469.zcurr;
  }
  struct zTriePath zkey;
  {
    sail_fixed_bytes_32 z2zE1468;
    {
      sail_fixed_bytes_20 z2zE1467;
      z2zE1467 = zentry.zaddr;
      z2zE1468 = zkeccak256_address(z2zE1467);
    }
    zkey = zpath_from_hash(z2zE1468);
  }
  bool z2zE1462;
  {
    bool z2zE1461;
    {
      bool z2zE1459;
      z2zE1459 = zcurrent.zpresent;
      z2zE1461 = not(z2zE1459);
    }
    bool z3zE1466;
    if (z2zE1461) {  z3zE1466 = true;  } else {
      struct zAccountInfo z2zE1460;
      z2zE1460 = zcurrent.zinfo;
      z3zE1466 = zaccount_info_empty(z2zE1460);
    }
    z2zE1462 = z3zE1466;
  }
  if (z2zE1462) {
    struct zTrieChange z2zE1463;
    CREATE(zTrieChange)(&z2zE1463);
    zTrieDelete(&z2zE1463, UNIT);
    struct zTrieUpdate z3zE1468;
    CREATE(zTrieUpdate)(&z3zE1468);
    COPY(zTrieChange)(&((&z3zE1468)->zchange), z2zE1463);
    z3zE1468.zkey = zkey;
    COPY(zTrieUpdate)((*(&z8zE510)), z3zE1468);
    KILL(zTrieUpdate)(&z3zE1468);
    KILL(zTrieChange)(&z2zE1463);
  } else {
    struct zTrieChange z2zE1466;
    CREATE(zTrieChange)(&z2zE1466);
    {
      struct zByteSliceFields z2zE1465;
      {
        struct zAccountInfo z2zE1464;
        z2zE1464 = zcurrent.zinfo;
        z2zE1465 = zencode_state_account(z2zE1464, zstorage_root);
      }
      zTriePut(&z2zE1466, z2zE1465);
    }
    struct zTrieUpdate z3zE1467;
    CREATE(zTrieUpdate)(&z3zE1467);
    COPY(zTrieChange)(&((&z3zE1467)->zchange), z2zE1466);
    z3zE1467.zkey = zkey;
    COPY(zTrieUpdate)((*(&z8zE510)), z3zE1467);
    KILL(zTrieUpdate)(&z3zE1467);
    KILL(zTrieChange)(&z2zE1466);
  }
end_function_1914: ;
  goto end_function_4031;
end_block_exception_1915: ;
  goto end_function_4031;
end_function_4031: ;
}

bool zaccount_value_changed(struct zAcctValue zvalue)
{
  bool z8zE511;
  bool z2zE1458;
  {
    bool z2zE1422;
    {
      uint64_t z2zE1420;
      {
        struct zAccountInfo z2zE1417;
        {
          struct zAccount z2zE1416;
          z2zE1416 = zvalue.zcurr;
          z2zE1417 = z2zE1416.zinfo;
        }
        z2zE1420 = z2zE1417.znonce;
      }
      uint64_t z2zE1421;
      {
        struct zAccountInfo z2zE1419;
        {
          struct zAccount z2zE1418;
          z2zE1418 = zvalue.zorig;
          z2zE1419 = z2zE1418.zinfo;
        }
        z2zE1421 = z2zE1419.znonce;
      }
      z2zE1422 = (z2zE1420 == z2zE1421);
    }
    z2zE1458 = not(z2zE1422);
  }
  bool z3zE1465;
  if (z2zE1458) {  z3zE1465 = true;  } else {
    bool z2zE1457;
    {
      bool z2zE1429;
      {
        sail_u256 z2zE1427;
        {
          struct zAccountInfo z2zE1424;
          {
            struct zAccount z2zE1423;
            z2zE1423 = zvalue.zcurr;
            z2zE1424 = z2zE1423.zinfo;
          }
          z2zE1427 = z2zE1424.zbalance;
        }
        sail_u256 z2zE1428;
        {
          struct zAccountInfo z2zE1426;
          {
            struct zAccount z2zE1425;
            z2zE1425 = zvalue.zorig;
            z2zE1426 = z2zE1425.zinfo;
          }
          z2zE1428 = z2zE1426.zbalance;
        }
        z2zE1429 = eq_u256(z2zE1427, z2zE1428);
      }
      z2zE1457 = not(z2zE1429);
    }
    bool z3zE1464;
    if (z2zE1457) {  z3zE1464 = true;  } else {
      bool z2zE1456;
      {
        bool z2zE1436;
        {
          sail_fixed_bytes_32 z2zE1434;
          {
            struct zAccountInfo z2zE1431;
            {
              struct zAccount z2zE1430;
              z2zE1430 = zvalue.zcurr;
              z2zE1431 = z2zE1430.zinfo;
            }
            z2zE1434 = z2zE1431.zstorage_root;
          }
          sail_fixed_bytes_32 z2zE1435;
          {
            struct zAccountInfo z2zE1433;
            {
              struct zAccount z2zE1432;
              z2zE1432 = zvalue.zorig;
              z2zE1433 = z2zE1432.zinfo;
            }
            z2zE1435 = z2zE1433.zstorage_root;
          }
          z2zE1436 = eq_fixed_bytes_32(z2zE1434, z2zE1435);
        }
        z2zE1456 = not(z2zE1436);
      }
      bool z3zE1463;
      if (z2zE1456) {  z3zE1463 = true;  } else {
        bool z2zE1455;
        {
          bool z2zE1443;
          {
            sail_fixed_bytes_32 z2zE1441;
            {
              struct zAccountInfo z2zE1438;
              {
                struct zAccount z2zE1437;
                z2zE1437 = zvalue.zcurr;
                z2zE1438 = z2zE1437.zinfo;
              }
              z2zE1441 = z2zE1438.zcode_hash;
            }
            sail_fixed_bytes_32 z2zE1442;
            {
              struct zAccountInfo z2zE1440;
              {
                struct zAccount z2zE1439;
                z2zE1439 = zvalue.zorig;
                z2zE1440 = z2zE1439.zinfo;
              }
              z2zE1442 = z2zE1440.zcode_hash;
            }
            z2zE1443 = eq_fixed_bytes_32(z2zE1441, z2zE1442);
          }
          z2zE1455 = not(z2zE1443);
        }
        bool z3zE1462;
        if (z2zE1455) {  z3zE1462 = true;  } else {
          bool z2zE1454;
          {
            bool z2zE1448;
            {
              bool z2zE1446;
              {
                struct zAccount z2zE1444;
                z2zE1444 = zvalue.zcurr;
                z2zE1446 = z2zE1444.zpresent;
              }
              bool z2zE1447;
              {
                struct zAccount z2zE1445;
                z2zE1445 = zvalue.zorig;
                z2zE1447 = z2zE1445.zpresent;
              }
              z2zE1448 = eq_bool(z2zE1446, z2zE1447);
            }
            z2zE1454 = not(z2zE1448);
          }
          bool z3zE1461;
          if (z2zE1454) {  z3zE1461 = true;  } else {
            bool z2zE1453;
            {
              bool z2zE1451;
              {
                struct zAccount z2zE1449;
                z2zE1449 = zvalue.zcurr;
                z2zE1451 = z2zE1449.zstorage_cleared;
              }
              bool z2zE1452;
              {
                struct zAccount z2zE1450;
                z2zE1450 = zvalue.zorig;
                z2zE1452 = z2zE1450.zstorage_cleared;
              }
              z2zE1453 = eq_bool(z2zE1451, z2zE1452);
            }
            z3zE1461 = not(z2zE1453);
          }
          z3zE1462 = z3zE1461;
        }
        z3zE1463 = z3zE1462;
      }
      z3zE1464 = z3zE1463;
    }
    z3zE1465 = z3zE1464;
  }
  z8zE511 = z3zE1465;
end_function_1912: ;
  return z8zE511;
end_block_exception_1913: ;

  return false;
}

bool zstorage_value_changed(struct zStorageValue zvalue)
{
  bool z8zE512;
  bool z2zE1415;
  {
    sail_u256 z2zE1413;
    z2zE1413 = zvalue.zcurr;
    sail_u256 z2zE1414;
    z2zE1414 = zvalue.zorig;
    z2zE1415 = eq_u256(z2zE1413, z2zE1414);
  }
  z8zE512 = not(z2zE1415);
end_function_1910: ;
  return z8zE512;
end_block_exception_1911: ;

  return false;
}

void znext_changed_storage_entry(struct zoptionzIRStorageEntryzK *z8zE513, sail_fixed_bytes_20 zaddr)
{
  bool zsearching;
  zsearching = true;
  struct zoptionzIRStorageEntryzK zresult;
  CREATE(zoptionzIRStorageEntryzK)(&zresult);
  zNonezIRStorageEntryzK(&zresult, UNIT);
  bool z3zE1458;
  unit z3zE1459;
while_1903: ;
  {
    z3zE1458 = zsearching;
    if (!(z3zE1458)) goto wend_1904;
    struct zoptionzIRStorageEntryzK z2zE1410;
    CREATE(zoptionzIRStorageEntryzK)(&z2zE1410);
    storage_block_iter_next(&z2zE1410, zaddr);
    unit z3zE1454;
    {
      if (z2zE1410.kind != Kind_zSomezIRStorageEntryzK) goto case_1907;
      struct zStorageEntry zentry;
      zentry = z2zE1410.variants.zSomezIRStorageEntryzK;
      bool z2zE1412;
      {
        struct zStorageValue z2zE1411;
        z2zE1411 = zentry.zvalue;
        z2zE1412 = zstorage_value_changed(z2zE1411);
      }
      if (z2zE1412) {
        zSomezIRStorageEntryzK(&zresult, zentry);
        unit z3zE1457;
        z3zE1457 = UNIT;
        zsearching = false;
        z3zE1454 = UNIT;
      } else {  z3zE1454 = UNIT;  }
      goto finish_match_1905;
    }
  case_1907: ;
    {
      /* complete */
      zsearching = false;
      z3zE1454 = UNIT;
      goto finish_match_1905;
    }
  case_1906: ;
  finish_match_1905: ;
    z3zE1459 = z3zE1454;
    KILL(zoptionzIRStorageEntryzK)(&z2zE1410);
    goto while_1903;
  }
wend_1904: ;
  unit z3zE1460;
  z3zE1460 = UNIT;
  COPY(zoptionzIRStorageEntryzK)((*(&z8zE513)), zresult);
  KILL(zoptionzIRStorageEntryzK)(&zresult);
end_function_1908: ;
  goto end_function_4030;
end_block_exception_1909: ;
  goto end_function_4030;
end_function_4030: ;
}

void zaccount_trie_update(struct ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9 *z8zE514, struct zAcctEntry zentry)
{
  sail_fixed_bytes_20 z2zE1402;
  z2zE1402 = zentry.zaddr;
  unit z3zE1448;
  z3zE1448 = storage_block_iter_begin(z2zE1402);
  bool zstorage_changed;
  {
    struct zoptionzIRStorageEntryzK z2zE1409;
    CREATE(zoptionzIRStorageEntryzK)(&z2zE1409);
    {
      sail_fixed_bytes_20 z2zE1408;
      z2zE1408 = zentry.zaddr;
      znext_changed_storage_entry(&z2zE1409, z2zE1408);
    }
    bool z3zE1449;
    {
      if (z2zE1409.kind != Kind_zSomezIRStorageEntryzK) goto case_1900;
      struct zStorageEntry z_storage_entry;
      z_storage_entry = z2zE1409.variants.zSomezIRStorageEntryzK;
      z3zE1449 = true;
      goto finish_match_1898;
    }
  case_1900: ;
    {
      /* complete */
      z3zE1449 = false;
      goto finish_match_1898;
    }
  case_1899: ;
  finish_match_1898: ;
    zstorage_changed = z3zE1449;
    KILL(zoptionzIRStorageEntryzK)(&z2zE1409);
  }
  sail_fixed_bytes_32 zstorage_root;
  {
    sail_fixed_bytes_20 z2zE1407;
    z2zE1407 = zentry.zaddr;
    zstorage_root = acct_post_storage_root_read(z2zE1407);
  }
  struct zTrieUpdate z2zE1405;
  CREATE(zTrieUpdate)(&z2zE1405);
  zaccount_update(&z2zE1405, zentry, zstorage_root);
  bool z2zE1406;
  {
    bool z2zE1404;
    {
      struct zAcctValue z2zE1403;
      z2zE1403 = zentry.zvalue;
      z2zE1404 = zaccount_value_changed(z2zE1403);
    }
    bool z3zE1452;
    if (z2zE1404) {  z3zE1452 = true;  } else {  z3zE1452 = zstorage_changed;  }
    z2zE1406 = z3zE1452;
  }
  struct ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9 z3zE1453;
  CREATE(ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9)(&z3zE1453);
  COPY(zTrieUpdate)(&((&z3zE1453)->ztup0), z2zE1405);
  z3zE1453.ztup1 = z2zE1406;
  COPY(ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9)((*(&z8zE514)), z3zE1453);
  KILL(ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9)(&z3zE1453);
  KILL(zTrieUpdate)(&z2zE1405);
end_function_1901: ;
  goto end_function_4029;
end_block_exception_1902: ;
  goto end_function_4029;
end_function_4029: ;
}

void znext_changed_account_trie_update(struct zoptionzIRTrieUpdatezK *z8zE515, unit z3zE1438)
{
  bool zsearching;
  zsearching = true;
  struct zoptionzIRTrieUpdatezK zresult;
  CREATE(zoptionzIRTrieUpdatezK)(&zresult);
  zNonezIRTrieUpdatezK(&zresult, UNIT);
  bool z3zE1445;
  unit z3zE1446;
while_1889: ;
  {
    z3zE1445 = zsearching;
    if (!(z3zE1445)) goto wend_1890;
    struct zoptionzIRAcctEntryzK z2zE1400;
    CREATE(zoptionzIRAcctEntryzK)(&z2zE1400);
    acct_block_iter_next(&z2zE1400, UNIT);
    unit z3zE1439;
    {
      if (z2zE1400.kind != Kind_zSomezIRAcctEntryzK) goto case_1893;
      struct zAcctEntry zentry;
      zentry = z2zE1400.variants.zSomezIRAcctEntryzK;
      struct ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9 z2zE1401;
      CREATE(ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9)(&z2zE1401);
      zaccount_trie_update(&z2zE1401, zentry);
      unit z3zE1441;
      {
        struct zTrieUpdate zupdate;
        CREATE(zTrieUpdate)(&zupdate);
        COPY(zTrieUpdate)(&zupdate, z2zE1401.ztup0);
        bool zchanged;
        zchanged = z2zE1401.ztup1;
        if (zchanged) {
          zSomezIRTrieUpdatezK(&zresult, zupdate);
          unit z3zE1443;
          z3zE1443 = UNIT;
          zsearching = false;
          z3zE1441 = UNIT;
        } else {  z3zE1441 = UNIT;  }
        KILL(zTrieUpdate)(&zupdate);
        goto finish_match_1894;
      }
    case_1895: ;
      sail_match_failure("next_changed_account_trie_update");
    finish_match_1894: ;
      z3zE1439 = z3zE1441;
      KILL(ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9)(&z2zE1401);
      goto finish_match_1891;
    }
  case_1893: ;
    {
      /* complete */
      zsearching = false;
      z3zE1439 = UNIT;
      goto finish_match_1891;
    }
  case_1892: ;
  finish_match_1891: ;
    z3zE1446 = z3zE1439;
    KILL(zoptionzIRAcctEntryzK)(&z2zE1400);
    goto while_1889;
  }
wend_1890: ;
  unit z3zE1447;
  z3zE1447 = UNIT;
  COPY(zoptionzIRTrieUpdatezK)((*(&z8zE515)), zresult);
  KILL(zoptionzIRTrieUpdatezK)(&zresult);
end_function_1896: ;
  goto end_function_4028;
end_block_exception_1897: ;
  goto end_function_4028;
end_function_4028: ;
}

struct zByteSliceFields zencode_storage_value(sail_u256 zvalue)
{
  struct zByteSliceFields z8zE516;
  uint64_t zencoded_len;
  zencoded_len = zrlp_uint_word_sizze(zvalue);
  uint64_t zstart;
  zstart = zscratch_begin(UNIT);
  unit z3zE1437;
  z3zE1437 = zrlp_write_uint_word(zvalue);
  z8zE516 = zrlp_finish(zstart);
end_function_1887: ;
  return z8zE516;
end_block_exception_1888: ;
  struct zByteSliceFields z8zE998 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE998;
}

void zstorage_update(struct zTrieUpdate *z8zE517, struct zStorageEntry zentry)
{
  struct zTriePath zkey;
  {
    sail_fixed_bytes_32 z2zE1399;
    {
      sail_u256 z2zE1398;
      {
        struct zStorageKey z2zE1397;
        z2zE1397 = zentry.zkey;
        z2zE1398 = z2zE1397.zslot;
      }
      z2zE1399 = zkeccak256_word(z2zE1398);
    }
    zkey = zpath_from_hash(z2zE1399);
  }
  struct zTrieChange zchange;
  CREATE(zTrieChange)(&zchange);
  {
    bool z2zE1393;
    {
      sail_u256 z2zE1392;
      {
        struct zStorageValue z2zE1391;
        z2zE1391 = zentry.zvalue;
        z2zE1392 = z2zE1391.zcurr;
      }
      z2zE1393 = zword_is_zzero(z2zE1392);
    }
    if (z2zE1393) {  zTrieDelete(&zchange, UNIT);  } else {
      struct zByteSliceFields z2zE1396;
      {
        sail_u256 z2zE1395;
        {
          struct zStorageValue z2zE1394;
          z2zE1394 = zentry.zvalue;
          z2zE1395 = z2zE1394.zcurr;
        }
        z2zE1396 = zencode_storage_value(z2zE1395);
      }
      zTriePut(&zchange, z2zE1396);
    }
  }
  struct zTrieUpdate z3zE1436;
  CREATE(zTrieUpdate)(&z3zE1436);
  COPY(zTrieChange)(&((&z3zE1436)->zchange), zchange);
  z3zE1436.zkey = zkey;
  COPY(zTrieUpdate)((*(&z8zE517)), z3zE1436);
  KILL(zTrieUpdate)(&z3zE1436);
  KILL(zTrieChange)(&zchange);
end_function_1885: ;
  goto end_function_4027;
end_block_exception_1886: ;
  goto end_function_4027;
end_function_4027: ;
}

void znext_storage_trie_update(struct zoptionzIRTrieUpdatezK *z8zE518, sail_fixed_bytes_20 zaddr)
{
  struct zoptionzIRStorageEntryzK z2zE1389;
  CREATE(zoptionzIRStorageEntryzK)(&z2zE1389);
  znext_changed_storage_entry(&z2zE1389, zaddr);
  struct zoptionzIRTrieUpdatezK z3zE1433;
  CREATE(zoptionzIRTrieUpdatezK)(&z3zE1433);
  {
    if (z2zE1389.kind != Kind_zSomezIRStorageEntryzK) goto case_1882;
    struct zStorageEntry zentry;
    zentry = z2zE1389.variants.zSomezIRStorageEntryzK;
    struct zTrieUpdate z2zE1390;
    CREATE(zTrieUpdate)(&z2zE1390);
    zstorage_update(&z2zE1390, zentry);
    zSomezIRTrieUpdatezK(&z3zE1433, z2zE1390);
    KILL(zTrieUpdate)(&z2zE1390);
    goto finish_match_1880;
  }
case_1882: ;
  {
    /* complete */
    zNonezIRTrieUpdatezK(&z3zE1433, UNIT);
    goto finish_match_1880;
  }
case_1881: ;
finish_match_1880: ;
  COPY(zoptionzIRTrieUpdatezK)((*(&z8zE518)), z3zE1433);
  KILL(zoptionzIRTrieUpdatezK)(&z3zE1433);
  KILL(zoptionzIRStorageEntryzK)(&z2zE1389);
end_function_1883: ;
  goto end_function_4026;
end_block_exception_1884: ;
  goto end_function_4026;
end_function_4026: ;
}

void ztrie_update_source_next(struct zoptionzIRTrieUpdatezK *z8zE519, struct zTrieUpdateSource zsource)
{
  struct zoptionzIRTrieUpdatezK z3zE1429;
  CREATE(zoptionzIRTrieUpdatezK)(&z3zE1429);
  {
    if (zsource.kind != Kind_zStorageTrieUpdates) goto case_1877;
    sail_fixed_bytes_20 zaddr;
    zaddr = zsource.variants.zStorageTrieUpdates;
    znext_storage_trie_update(&z3zE1429, zaddr);
    goto finish_match_1874;
  }
case_1877: ;
  {
    if (zsource.kind != Kind_zChangedAccountTrieUpdates) goto case_1876;
    znext_changed_account_trie_update(&z3zE1429, UNIT);
    goto finish_match_1874;
  }
case_1876: ;
  {
    /* complete */
    zcached_account_trie_update_next(&z3zE1429, UNIT);
    goto finish_match_1874;
  }
case_1875: ;
finish_match_1874: ;
  COPY(zoptionzIRTrieUpdatezK)((*(&z8zE519)), z3zE1429);
  KILL(zoptionzIRTrieUpdatezK)(&z3zE1429);
end_function_1878: ;
  goto end_function_4025;
end_block_exception_1879: ;
  goto end_function_4025;
end_function_4025: ;
}

void ztrie_updates_begin(struct zTrieUpdateCursor *z8zE520, struct zTrieUpdateSource zsource)
{
  struct zoptionzIRTrieUpdatezK z2zE1388;
  CREATE(zoptionzIRTrieUpdatezK)(&z2zE1388);
  ztrie_update_source_next(&z2zE1388, zsource);
  struct zTrieUpdateCursor z3zE1428;
  CREATE(zTrieUpdateCursor)(&z3zE1428);
  COPY(zoptionzIRTrieUpdatezK)(&((&z3zE1428)->zpending), z2zE1388);
  COPY(zTrieUpdateSource)(&((&z3zE1428)->zsource), zsource);
  COPY(zTrieUpdateCursor)((*(&z8zE520)), z3zE1428);
  KILL(zTrieUpdateCursor)(&z3zE1428);
  KILL(zoptionzIRTrieUpdatezK)(&z2zE1388);
end_function_1872: ;
  goto end_function_4024;
end_block_exception_1873: ;
  goto end_function_4024;
end_function_4024: ;
}

bool zupdates_empty(struct zTrieUpdateCursor zupdates)
{
  bool z8zE521;
  struct zoptionzIRTrieUpdatezK z2zE1387;
  CREATE(zoptionzIRTrieUpdatezK)(&z2zE1387);
  COPY(zoptionzIRTrieUpdatezK)(&z2zE1387, zupdates.zpending);
  bool z3zE1425;
  {
    if (z2zE1387.kind != Kind_zNonezIRTrieUpdatezK) goto case_1869;
    z3zE1425 = true;
    goto finish_match_1867;
  }
case_1869: ;
  {
    /* complete */
    z3zE1425 = false;
    goto finish_match_1867;
  }
case_1868: ;
finish_match_1867: ;
  z8zE521 = z3zE1425;
  KILL(zoptionzIRTrieUpdatezK)(&z2zE1387);
end_function_1870: ;
  return z8zE521;
end_block_exception_1871: ;

  return false;
}

void ztrie_updates_advance(struct zTrieUpdateCursor *z8zE522, struct zTrieUpdateCursor zupdates)
{
  struct zTrieUpdateSource z2zE1384;
  CREATE(zTrieUpdateSource)(&z2zE1384);
  COPY(zTrieUpdateSource)(&z2zE1384, zupdates.zsource);
  struct zoptionzIRTrieUpdatezK z2zE1386;
  CREATE(zoptionzIRTrieUpdatezK)(&z2zE1386);
  {
    struct zTrieUpdateSource z2zE1385;
    CREATE(zTrieUpdateSource)(&z2zE1385);
    COPY(zTrieUpdateSource)(&z2zE1385, zupdates.zsource);
    ztrie_update_source_next(&z2zE1386, z2zE1385);
    KILL(zTrieUpdateSource)(&z2zE1385);
  }
  struct zTrieUpdateCursor z3zE1424;
  CREATE(zTrieUpdateCursor)(&z3zE1424);
  COPY(zoptionzIRTrieUpdatezK)(&((&z3zE1424)->zpending), z2zE1386);
  COPY(zTrieUpdateSource)(&((&z3zE1424)->zsource), z2zE1384);
  COPY(zTrieUpdateCursor)((*(&z8zE522)), z3zE1424);
  KILL(zTrieUpdateCursor)(&z3zE1424);
  KILL(zoptionzIRTrieUpdatezK)(&z2zE1386);
  KILL(zTrieUpdateSource)(&z2zE1384);
end_function_1865: ;
  goto end_function_4023;
end_block_exception_1866: ;
  goto end_function_4023;
end_function_4023: ;
}

bool znext_update_under(struct zTrieUpdateCursor zupdates, struct zTriePath zprefix)
{
  bool z8zE523;
  struct zoptionzIRTrieUpdatezK z2zE1382;
  CREATE(zoptionzIRTrieUpdatezK)(&z2zE1382);
  COPY(zoptionzIRTrieUpdatezK)(&z2zE1382, zupdates.zpending);
  bool z3zE1421;
  {
    if (z2zE1382.kind != Kind_zSomezIRTrieUpdatezK) goto case_1862;
    struct zTrieUpdate zupdate;
    CREATE(zTrieUpdate)(&zupdate);
    COPY(zTrieUpdate)(&zupdate, z2zE1382.variants.zSomezIRTrieUpdatezK);
    struct zTriePath z2zE1383;
    z2zE1383 = zupdate.zkey;
    {
      z3zE1421 = zpath_prefix_of(zprefix, z2zE1383);
      if (have_exception) {
        KILL(zoptionzIRTrieUpdatezK)(&z2zE1382);
        KILL(zTrieUpdate)(&zupdate);
        goto end_block_exception_1864;
      }
    }
    KILL(zTrieUpdate)(&zupdate);
    goto finish_match_1860;
  }
case_1862: ;
  {
    /* complete */
    z3zE1421 = false;
    goto finish_match_1860;
  }
case_1861: ;
finish_match_1860: ;
  z8zE523 = z3zE1421;
  KILL(zoptionzIRTrieUpdatezK)(&z2zE1382);
end_function_1863: ;
  return z8zE523;
end_block_exception_1864: ;

  return false;
}

void zitem_leaf(struct zTrieItem *z8zE524, struct zTriePath zpath, struct zByteSliceFields zvalue)
{
  struct zTrieItemValue z2zE1381;
  CREATE(zTrieItemValue)(&z2zE1381);
  zLeafItem(&z2zE1381, zvalue);
  struct zTrieItem z3zE1420;
  CREATE(zTrieItem)(&z3zE1420);
  z3zE1420.zpath = zpath;
  COPY(zTrieItemValue)(&((&z3zE1420)->zvalue), z2zE1381);
  COPY(zTrieItem)((*(&z8zE524)), z3zE1420);
  KILL(zTrieItem)(&z3zE1420);
  KILL(zTrieItemValue)(&z2zE1381);
end_function_1858: ;
  goto end_function_4022;
end_block_exception_1859: ;
  goto end_function_4022;
end_function_4022: ;
}

void zitem_branch(struct zTrieItem *z8zE525, struct zTriePath zpath, struct zNodeRef zchildref)
{
  struct zTrieItemValue z2zE1380;
  CREATE(zTrieItemValue)(&z2zE1380);
  zBranchItem(&z2zE1380, zchildref);
  struct zTrieItem z3zE1419;
  CREATE(zTrieItem)(&z3zE1419);
  z3zE1419.zpath = zpath;
  COPY(zTrieItemValue)(&((&z3zE1419)->zvalue), z2zE1380);
  COPY(zTrieItem)((*(&z8zE525)), z3zE1419);
  KILL(zTrieItem)(&z3zE1419);
  KILL(zTrieItemValue)(&z2zE1380);
end_function_1856: ;
  goto end_function_4021;
end_block_exception_1857: ;
  goto end_function_4021;
end_function_4021: ;
}

void zitem_subtree(struct zTrieItem *z8zE526, struct zTriePath zpath, struct zNodeRef zchildref)
{
  struct zTrieItemValue z2zE1379;
  CREATE(zTrieItemValue)(&z2zE1379);
  zSubtreeItem(&z2zE1379, zchildref);
  struct zTrieItem z3zE1418;
  CREATE(zTrieItem)(&z3zE1418);
  z3zE1418.zpath = zpath;
  COPY(zTrieItemValue)(&((&z3zE1418)->zvalue), z2zE1379);
  COPY(zTrieItem)((*(&z8zE526)), z3zE1418);
  KILL(zTrieItem)(&z3zE1418);
  KILL(zTrieItemValue)(&z2zE1379);
end_function_1854: ;
  goto end_function_4020;
end_block_exception_1855: ;
  goto end_function_4020;
end_function_4020: ;
}

void zitem_ref(struct zNodeRef *z8zE527, struct zTrieItem zit, uint64_t zdepth)
{
  struct zTriePath zsuffix;
  {
    struct zTriePath z2zE1378;
    z2zE1378 = zit.zpath;
    {
      zsuffix = zpath_drop(z2zE1378, zdepth);
      if (have_exception) {  goto end_block_exception_1853;  }
    }
  }
  struct zTrieItemValue z2zE1370;
  CREATE(zTrieItemValue)(&z2zE1370);
  COPY(zTrieItemValue)(&z2zE1370, zit.zvalue);
  struct zNodeRef z3zE1410;
  CREATE(zNodeRef)(&z3zE1410);
  {
    if (z2zE1370.kind != Kind_zLeafItem) goto case_1851;
    struct zByteSliceFields zvalue;
    zvalue = z2zE1370.variants.zLeafItem;
    {
      zleaf_child_ref(&z3zE1410, zsuffix, zvalue);
      if (have_exception) {
        KILL(zTrieItemValue)(&z2zE1370);
        KILL(zNodeRef)(&z3zE1410);
        goto end_block_exception_1853;
      }
    }
    goto finish_match_1845;
  }
case_1851: ;
  {
    if (z2zE1370.kind != Kind_zBranchItem) goto case_1850;
    struct zNodeRef zsubref;
    CREATE(zNodeRef)(&zsubref);
    COPY(zNodeRef)(&zsubref, z2zE1370.variants.zBranchItem);
    bool z2zE1372;
    {
      uint64_t z2zE1371;
      z2zE1371 = zpath_len(zsuffix);
      z2zE1372 = (z2zE1371 == UINT64_C(0));
    }
    if (z2zE1372) {  COPY(zNodeRef)(&z3zE1410, zsubref);  } else {
      {
        zmerge_ext_ref(&z3zE1410, zsuffix, zsubref);
        if (have_exception) {
          KILL(zTrieItemValue)(&z2zE1370);
          KILL(zNodeRef)(&z3zE1410);
          KILL(zNodeRef)(&zsubref);
          goto end_block_exception_1853;
        }
      }
    }
    KILL(zNodeRef)(&zsubref);
    goto finish_match_1845;
  }
case_1850: ;
  {
    /* complete */
    struct zNodeRef z3zE1417;
    CREATE(zNodeRef)(&z3zE1417);
    COPY(zNodeRef)(&z3zE1417, z2zE1370.variants.zSubtreeItem);
    bool z2zE1374;
    {
      uint64_t z2zE1373;
      z2zE1373 = zpath_len(zsuffix);
      z2zE1374 = (z2zE1373 == UINT64_C(0));
    }
    if (z2zE1374) {  COPY(zNodeRef)(&z3zE1410, z3zE1417);  } else {
      struct zNodeRef z3zE1412;
      CREATE(zNodeRef)(&z3zE1412);
      {
        if (z3zE1417.kind != Kind_zHashRef) goto case_1849;
        sail_fixed_bytes_32 zh;
        zh = z3zE1417.variants.zHashRef;
        struct zByteSliceFields znode;
        znode = znode_db_lookup(zh);
        bool z2zE1376;
        {
          uint64_t z2zE1375;
          z2zE1375 = znode.zlen;
          z2zE1376 = (z2zE1375 == UINT64_C(0));
        }
        if (z2zE1376) {
          struct zexception z2zE1377;
          CREATE(zexception)(&z2zE1377);
          zInvalidBlock(&z2zE1377, zWitnessDeficient);
          COPY(zexception)(current_exception, z2zE1377);
          have_exception = true;
          COPY(sail_string)(throw_location, "sail/lib/mpt/updates.sail:117.24-117.60");
          KILL(zTrieItemValue)(&z2zE1370);
          KILL(zNodeRef)(&z3zE1410);
          KILL(zNodeRef)(&z3zE1417);
          KILL(zNodeRef)(&z3zE1412);
          KILL(zexception)(&z2zE1377);
          goto end_block_exception_1853;
          /* unreachable after throw */
          KILL(zexception)(&z2zE1377);
        } else {
          {
            zmerge_ext_node(&z3zE1412, zsuffix, znode);
            if (have_exception) {
              KILL(zTrieItemValue)(&z2zE1370);
              KILL(zNodeRef)(&z3zE1410);
              KILL(zNodeRef)(&z3zE1417);
              KILL(zNodeRef)(&z3zE1412);
              goto end_block_exception_1853;
            }
          }
        }
        goto finish_match_1847;
      }
    case_1849: ;
      {
        {
          zmerge_ext_ref(&z3zE1412, zsuffix, z3zE1417);
          if (have_exception) {
            KILL(zTrieItemValue)(&z2zE1370);
            KILL(zNodeRef)(&z3zE1410);
            KILL(zNodeRef)(&z3zE1417);
            KILL(zNodeRef)(&z3zE1412);
            goto end_block_exception_1853;
          }
        }
        goto finish_match_1847;
      }
    case_1848: ;
    finish_match_1847: ;
      COPY(zNodeRef)(&z3zE1410, z3zE1412);
      KILL(zNodeRef)(&z3zE1412);
    }
    KILL(zNodeRef)(&z3zE1417);
    goto finish_match_1845;
  }
case_1846: ;
finish_match_1845: ;
  COPY(zNodeRef)((*(&z8zE527)), z3zE1410);
  KILL(zNodeRef)(&z3zE1410);
  KILL(zTrieItemValue)(&z2zE1370);
end_function_1852: ;
  goto end_function_4019;
end_block_exception_1853: ;
  goto end_function_4019;
end_function_4019: ;
}

void zempty_trie_branch_frame(struct zTrieBranchFrame *z8zE528, uint64_t zdepth)
{
  zz5vecz8z5unionz0zzNodeRefz9 z2zE1369;
  CREATE(zz5vecz8z5unionz0zzNodeRefz9)(&z2zE1369);
  {
    struct zNodeRef z2zE1368;
    CREATE(zNodeRef)(&z2zE1368);
    zEmptyRef(&z2zE1368, UNIT);
    fast_unsigned_vector_init_zz5vecz8z5unionz0zzNodeRefz9(&z2zE1369, UINT64_C(16), z2zE1368);
    KILL(zNodeRef)(&z2zE1368);
  }
  struct zTrieBranchFrame z3zE1409;
  CREATE(zTrieBranchFrame)(&z3zE1409);
  COPY(zz5vecz8z5unionz0zzNodeRefz9)(&((&z3zE1409)->zchildren), z2zE1369);
  z3zE1409.zdepth = zdepth;
  z3zE1409.zmask = UINT64_C(0x0000);
  COPY(zTrieBranchFrame)((*(&z8zE528)), z3zE1409);
  KILL(zTrieBranchFrame)(&z3zE1409);
  KILL(zz5vecz8z5unionz0zzNodeRefz9)(&z2zE1369);
end_function_1843: ;
  goto end_function_4018;
end_block_exception_1844: ;
  goto end_function_4018;
end_function_4018: ;
}

void ztrie_builder_empty(struct zTrieBuilder *z8zE529, unit z3zE1406)
{
  struct zNodeRef z2zE1367;
  CREATE(zNodeRef)(&z2zE1367);
  zEmptyRef(&z2zE1367, UNIT);
  struct zTrieBuilder z3zE1407;
  CREATE(zTrieBuilder)(&z3zE1407);
  z3zE1407.zcomplete = false;
  zz5listz8z5structz0zzTrieBranchFramez9 z3zE1408;
  CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z3zE1408);
  COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&((&z3zE1407)->zframes), z3zE1408);
  KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z3zE1408);
  COPY(zNodeRef)(&((&z3zE1407)->zroot), z2zE1367);
  COPY(zTrieBuilder)((*(&z8zE529)), z3zE1407);
  KILL(zTrieBuilder)(&z3zE1407);
  KILL(zNodeRef)(&z2zE1367);
end_function_1841: ;
  goto end_function_4017;
end_block_exception_1842: ;
  goto end_function_4017;
end_function_4017: ;
}

void ztrie_builder_push(struct zTrieBuilder *z8zE530, struct zTrieBuilder zbuilder, uint64_t zdepth)
{
  zz5listz8z5structz0zzTrieBranchFramez9 z2zE1364;
  CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1364);
  {
    struct zTrieBranchFrame z2zE1362;
    CREATE(zTrieBranchFrame)(&z2zE1362);
    zempty_trie_branch_frame(&z2zE1362, zdepth);
    zz5listz8z5structz0zzTrieBranchFramez9 z2zE1363;
    CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1363);
    COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1363, zbuilder.zframes);
    zconsz3z5structz0zzTrieBranchFrame(&z2zE1364, z2zE1362, z2zE1363);
    KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1363);
    KILL(zTrieBranchFrame)(&z2zE1362);
  }
  struct zNodeRef z2zE1365;
  CREATE(zNodeRef)(&z2zE1365);
  COPY(zNodeRef)(&z2zE1365, zbuilder.zroot);
  bool z2zE1366;
  z2zE1366 = zbuilder.zcomplete;
  struct zTrieBuilder z3zE1405;
  CREATE(zTrieBuilder)(&z3zE1405);
  z3zE1405.zcomplete = z2zE1366;
  COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&((&z3zE1405)->zframes), z2zE1364);
  COPY(zNodeRef)(&((&z3zE1405)->zroot), z2zE1365);
  COPY(zTrieBuilder)((*(&z8zE530)), z3zE1405);
  KILL(zTrieBuilder)(&z3zE1405);
  KILL(zNodeRef)(&z2zE1365);
  KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1364);
end_function_1839: ;
  goto end_function_4016;
end_block_exception_1840: ;
  goto end_function_4016;
end_function_4016: ;
}

void ztrie_builder_attach(struct zTrieBuilder *z8zE531, struct zTrieBuilder zbuilder, struct zTriePath zpath, struct zNodeRef zchild)
{
  zz5listz8z5structz0zzTrieBranchFramez9 z2zE1347;
  CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1347);
  COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1347, zbuilder.zframes);
  struct zTrieBuilder z3zE1396;
  CREATE(zTrieBuilder)(&z3zE1396);
  {
    if (!((z2zE1347 == NULL))) goto case_1836;
    struct zexception z2zE1348;
    CREATE(zexception)(&z2zE1348);
    zInvalidBlock(&z2zE1348, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE1348);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/updates.sail:181.16-181.52");
    KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1347);
    KILL(zTrieBuilder)(&z3zE1396);
    KILL(zexception)(&z2zE1348);
    goto end_block_exception_1838;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1348);
    goto finish_match_1834;
  }
case_1836: ;
  {
    /* complete */
    struct zTrieBranchFrame zcurrent;
    CREATE(zTrieBranchFrame)(&zcurrent);
    COPY(zTrieBranchFrame)(&zcurrent, (*z2zE1347).hd);
    zz5listz8z5structz0zzTrieBranchFramez9 zrest;
    CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&zrest);
    COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&zrest, (*z2zE1347).tl);
    struct zTrieBranchFrame zframe;
    CREATE(zTrieBranchFrame)(&zframe);
    COPY(zTrieBranchFrame)(&zframe, zcurrent);
    uint64_t zdepth;
    zdepth = zframe.zdepth;
    bool z2zE1350;
    {
      uint64_t z2zE1349;
      z2zE1349 = zpath_len(zpath);
      z2zE1350 = (!(zdepth < z2zE1349));
    }
    unit z3zE1397;
    if (z2zE1350) {
      struct zexception z2zE1351;
      CREATE(zexception)(&z2zE1351);
      zInvalidBlock(&z2zE1351, zWitnessDeficient);
      COPY(zexception)(current_exception, z2zE1351);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/lib/mpt/updates.sail:186.16-186.52");
      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1347);
      KILL(zTrieBuilder)(&z3zE1396);
      KILL(zTrieBranchFrame)(&zcurrent);
      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&zrest);
      KILL(zTrieBranchFrame)(&zframe);
      KILL(zexception)(&z2zE1351);
      goto end_block_exception_1838;
      /* unreachable after throw */
      KILL(zexception)(&z2zE1351);
    } else {  z3zE1397 = UNIT;  }
    uint64_t zchild_index;
    zchild_index = zpath_nibble(zpath, zdepth);
    bool z2zE1353;
    {
      uint64_t z2zE1352;
      z2zE1352 = zframe.zmask;
      z2zE1353 = zbranch_mask_has(z2zE1352, zchild_index);
    }
    unit z3zE1401;
    if (z2zE1353) {
      struct zexception z2zE1354;
      CREATE(zexception)(&z2zE1354);
      zInvalidBlock(&z2zE1354, zWitnessDeficient);
      COPY(zexception)(current_exception, z2zE1354);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/lib/mpt/updates.sail:190.16-190.52");
      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1347);
      KILL(zTrieBuilder)(&z3zE1396);
      KILL(zTrieBranchFrame)(&zcurrent);
      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&zrest);
      KILL(zTrieBranchFrame)(&zframe);
      KILL(zexception)(&z2zE1354);
      goto end_block_exception_1838;
      /* unreachable after throw */
      KILL(zexception)(&z2zE1354);
    } else {  z3zE1401 = UNIT;  }
    uint64_t z2zE1355;
    z2zE1355 = zframe.zmask;
    zframe.zmask = zbranch_mask_set(z2zE1355, zchild_index);
    unit z3zE1400;
    z3zE1400 = UNIT;
    zz5vecz8z5unionz0zzNodeRefz9 z2zE1358;
    CREATE(zz5vecz8z5unionz0zzNodeRefz9)(&z2zE1358);
    {
      zz5vecz8z5unionz0zzNodeRefz9 z2zE1356;
      CREATE(zz5vecz8z5unionz0zzNodeRefz9)(&z2zE1356);
      COPY(zz5vecz8z5unionz0zzNodeRefz9)(&z2zE1356, zframe.zchildren);
      uint64_t z2zE1357;
      z2zE1357 = ((uint64_t) zchild_index);
      fast_unsigned_vector_update_zz5vecz8z5unionz0zzNodeRefz9(&z2zE1358, z2zE1356, z2zE1357, zchild);
      KILL(zz5vecz8z5unionz0zzNodeRefz9)(&z2zE1356);
    }
    COPY(zz5vecz8z5unionz0zzNodeRefz9)(&((&zframe)->zchildren), z2zE1358);
    unit z3zE1399;
    z3zE1399 = UNIT;
    zz5listz8z5structz0zzTrieBranchFramez9 z2zE1359;
    CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1359);
    zconsz3z5structz0zzTrieBranchFrame(&z2zE1359, zframe, zrest);
    struct zNodeRef z2zE1360;
    CREATE(zNodeRef)(&z2zE1360);
    COPY(zNodeRef)(&z2zE1360, zbuilder.zroot);
    bool z2zE1361;
    z2zE1361 = zbuilder.zcomplete;
    z3zE1396.zcomplete = z2zE1361;
    COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&((&z3zE1396)->zframes), z2zE1359);
    COPY(zNodeRef)(&((&z3zE1396)->zroot), z2zE1360);
    goto finish_match_1834;
  }
case_1835: ;
finish_match_1834: ;
  COPY(zTrieBuilder)((*(&z8zE531)), z3zE1396);
  KILL(zTrieBuilder)(&z3zE1396);
  KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1347);
end_function_1837: ;
  goto end_function_4015;
end_block_exception_1838: ;
  goto end_function_4015;
end_function_4015: ;
}

void ztrie_builder_pop(struct ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9 *z8zE532, struct zTrieBuilder zbuilder)
{
  zz5listz8z5structz0zzTrieBranchFramez9 z2zE1342;
  CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1342);
  COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1342, zbuilder.zframes);
  struct ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9 z3zE1391;
  CREATE(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z3zE1391);
  {
    if (!((z2zE1342 == NULL))) goto case_1831;
    struct zexception z2zE1343;
    CREATE(zexception)(&z2zE1343);
    zInvalidBlock(&z2zE1343, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE1343);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/updates.sail:201.16-201.52");
    KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1342);
    KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z3zE1391);
    KILL(zexception)(&z2zE1343);
    goto end_block_exception_1833;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1343);
    goto finish_match_1829;
  }
case_1831: ;
  {
    /* complete */
    struct zTrieBranchFrame zframe;
    CREATE(zTrieBranchFrame)(&zframe);
    COPY(zTrieBranchFrame)(&zframe, (*z2zE1342).hd);
    zz5listz8z5structz0zzTrieBranchFramez9 zrest;
    CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&zrest);
    COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&zrest, (*z2zE1342).tl);
    struct zTrieBuilder z2zE1346;
    CREATE(zTrieBuilder)(&z2zE1346);
    {
      struct zNodeRef z2zE1344;
      CREATE(zNodeRef)(&z2zE1344);
      COPY(zNodeRef)(&z2zE1344, zbuilder.zroot);
      bool z2zE1345;
      z2zE1345 = zbuilder.zcomplete;
      z2zE1346.zcomplete = z2zE1345;
      COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&((&z2zE1346)->zframes), zrest);
      COPY(zNodeRef)(&((&z2zE1346)->zroot), z2zE1344);
      KILL(zNodeRef)(&z2zE1344);
    }
    COPY(zTrieBranchFrame)(&((&z3zE1391)->ztup0), zframe);
    COPY(zTrieBuilder)(&((&z3zE1391)->ztup1), z2zE1346);
    KILL(zTrieBuilder)(&z2zE1346);
    KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&zrest);
    KILL(zTrieBranchFrame)(&zframe);
    goto finish_match_1829;
  }
case_1830: ;
finish_match_1829: ;
  COPY(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)((*(&z8zE532)), z3zE1391);
  KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z3zE1391);
  KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1342);
end_function_1832: ;
  goto end_function_4014;
end_block_exception_1833: ;
  goto end_function_4014;
end_function_4014: ;
}

void ztrie_builder_wrap_branch(struct zNodeRef *z8zE533, struct zTriePath zanchor, uint64_t zparent_depth, uint64_t zchild_depth, struct zNodeRef zchild)
{
  uint64_t zchild_start;
  {    zchild_start = (zparent_depth + UINT64_C(1));
  }
  bool z2zE1339;
  z2zE1339 = (!(zchild_start < zchild_depth));
  if (z2zE1339) {  COPY(zNodeRef)((*(&z8zE533)), zchild);  } else {
    uint64_t zgap;
    {    zgap = (zchild_depth - zchild_start);
    }
    struct zTriePath z2zE1341;
    {
      struct zTriePath z2zE1340;
      {
        z2zE1340 = zpath_drop(zanchor, zchild_start);
        if (have_exception) {  goto end_block_exception_1828;  }
      }
      {
        z2zE1341 = zpath_take(z2zE1340, zgap);
        if (have_exception) {  goto end_block_exception_1828;  }
      }
    }
    {
      zextension_child_ref((*(&z8zE533)), z2zE1341, zchild);
      if (have_exception) {  goto end_block_exception_1828;  }
    }
  }
end_function_1827: ;
  goto end_function_4013;
end_block_exception_1828: ;
  goto end_function_4013;
end_function_4013: ;
}

void ztrie_builder_close(struct zTrieBuilder *z8zE534, struct zTrieBuilder zbuilder, struct zTriePath zanchor, struct zoptionzIU64zK znext_common, uint64_t zfuel)
{
  bool z2zE1314;
  z2zE1314 = (zfuel == UINT64_C(0));
  if (z2zE1314) {
    struct zexception z2zE1315;
    CREATE(zexception)(&z2zE1315);
    zInvalidBlock(&z2zE1315, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE1315);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/updates.sail:234.8-234.44");
    KILL(zexception)(&z2zE1315);
    goto end_block_exception_1826;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1315);
  } else {
    zz5listz8z5structz0zzTrieBranchFramez9 z2zE1316;
    CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1316);
    COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1316, zbuilder.zframes);
    struct zTrieBuilder z3zE1370;
    CREATE(zTrieBuilder)(&z3zE1370);
    {
      if (!((z2zE1316 == NULL))) goto case_1824;
      COPY(zTrieBuilder)(&z3zE1370, zbuilder);
      goto finish_match_1808;
    }
  case_1824: ;
    {
      /* complete */
      struct zTrieBranchFrame ztop;
      CREATE(zTrieBranchFrame)(&ztop);
      COPY(zTrieBranchFrame)(&ztop, (*z2zE1316).hd);
      bool zshould_close;
      {
        bool z3zE1371;
        {
          if (znext_common.kind != Kind_zNonezIU64zK) goto case_1812;
          z3zE1371 = true;
          goto finish_match_1810;
        }
      case_1812: ;
        {
          /* complete */
          uint64_t zdepth;
          zdepth = znext_common.variants.zSomezIU64zK;
          uint64_t z2zE1338;
          z2zE1338 = ztop.zdepth;
          z3zE1371 = (zdepth < z2zE1338);
          goto finish_match_1810;
        }
      case_1811: ;
      finish_match_1810: ;
        zshould_close = z3zE1371;
      }
      bool z2zE1317;
      z2zE1317 = not(zshould_close);
      if (z2zE1317) {  COPY(zTrieBuilder)(&z3zE1370, zbuilder);  } else {
        struct ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9 z2zE1318;
        CREATE(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE1318);
        {
          ztrie_builder_pop(&z2zE1318, zbuilder);
          if (have_exception) {
            KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1316);
            KILL(zTrieBuilder)(&z3zE1370);
            KILL(zTrieBranchFrame)(&ztop);
            KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE1318);
            goto end_block_exception_1826;
          }
        }
        struct zTrieBuilder z3zE1375;
        CREATE(zTrieBuilder)(&z3zE1375);
        {
          struct zTrieBranchFrame zframe;
          CREATE(zTrieBranchFrame)(&zframe);
          COPY(zTrieBranchFrame)(&zframe, z2zE1318.ztup0);
          struct zTrieBuilder zpopped;
          CREATE(zTrieBuilder)(&zpopped);
          COPY(zTrieBuilder)(&zpopped, z2zE1318.ztup1);
          struct zNodeRef zchild;
          CREATE(zNodeRef)(&zchild);
          {
            uint64_t z2zE1336;
            z2zE1336 = zframe.zmask;
            zz5vecz8z5unionz0zzNodeRefz9 z2zE1337;
            CREATE(zz5vecz8z5unionz0zzNodeRefz9)(&z2zE1337);
            COPY(zz5vecz8z5unionz0zzNodeRefz9)(&z2zE1337, zframe.zchildren);
            {
              zbranch_child_ref(&zchild, z2zE1336, z2zE1337);
              if (have_exception) {
                KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1316);
                KILL(zTrieBuilder)(&z3zE1370);
                KILL(zTrieBranchFrame)(&ztop);
                KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE1318);
                KILL(zTrieBuilder)(&z3zE1375);
                KILL(zTrieBranchFrame)(&zframe);
                KILL(zTrieBuilder)(&zpopped);
                KILL(zNodeRef)(&zchild);
                KILL(zz5vecz8z5unionz0zzNodeRefz9)(&z2zE1337);
                goto end_block_exception_1826;
              }
            }
            KILL(zz5vecz8z5unionz0zzNodeRefz9)(&z2zE1337);
          }
          struct zTrieBuilder zwith_parent;
          CREATE(zTrieBuilder)(&zwith_parent);
          {
            zz5listz8z5structz0zzTrieBranchFramez9 z2zE1320;
            CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1320);
            COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1320, zpopped.zframes);
            struct zTrieBuilder z3zE1376;
            CREATE(zTrieBuilder)(&z3zE1376);
            {
              if ((z2zE1320 == NULL)) goto case_1820;
              struct zTrieBranchFrame zparent;
              CREATE(zTrieBranchFrame)(&zparent);
              COPY(zTrieBranchFrame)(&zparent, (*z2zE1320).hd);
              struct zTrieBuilder z3zE1382;
              CREATE(zTrieBuilder)(&z3zE1382);
              {
                if (znext_common.kind != Kind_zSomezIU64zK) goto case_1823;
                uint64_t zcommon;
                zcommon = znext_common.variants.zSomezIU64zK;
                bool z2zE1322;
                {
                  uint64_t z2zE1321;
                  z2zE1321 = zparent.zdepth;
                  z2zE1322 = (z2zE1321 < zcommon);
                }
                if (z2zE1322) {
                  struct zTrieBuilder zintermediate;
                  CREATE(zTrieBuilder)(&zintermediate);
                  ztrie_builder_push(&zintermediate, zpopped, zcommon);
                  struct zNodeRef z2zE1324;
                  CREATE(zNodeRef)(&z2zE1324);
                  {
                    uint64_t z2zE1323;
                    z2zE1323 = zframe.zdepth;
                    {
                      ztrie_builder_wrap_branch(&z2zE1324, zanchor, zcommon, z2zE1323, zchild);
                      if (have_exception) {
                        KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1316);
                        KILL(zTrieBuilder)(&z3zE1370);
                        KILL(zTrieBranchFrame)(&ztop);
                        KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE1318);
                        KILL(zTrieBuilder)(&z3zE1375);
                        KILL(zTrieBranchFrame)(&zframe);
                        KILL(zTrieBuilder)(&zpopped);
                        KILL(zNodeRef)(&zchild);
                        KILL(zTrieBuilder)(&zwith_parent);
                        KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1320);
                        KILL(zTrieBuilder)(&z3zE1376);
                        KILL(zTrieBranchFrame)(&zparent);
                        KILL(zTrieBuilder)(&z3zE1382);
                        KILL(zTrieBuilder)(&zintermediate);
                        KILL(zNodeRef)(&z2zE1324);
                        goto end_block_exception_1826;
                      }
                    }
                  }
                  {
                    ztrie_builder_attach(&z3zE1382, zintermediate, zanchor, z2zE1324);
                    if (have_exception) {
                      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1316);
                      KILL(zTrieBuilder)(&z3zE1370);
                      KILL(zTrieBranchFrame)(&ztop);
                      KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE1318);
                      KILL(zTrieBuilder)(&z3zE1375);
                      KILL(zTrieBranchFrame)(&zframe);
                      KILL(zTrieBuilder)(&zpopped);
                      KILL(zNodeRef)(&zchild);
                      KILL(zTrieBuilder)(&zwith_parent);
                      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1320);
                      KILL(zTrieBuilder)(&z3zE1376);
                      KILL(zTrieBranchFrame)(&zparent);
                      KILL(zTrieBuilder)(&z3zE1382);
                      KILL(zTrieBuilder)(&zintermediate);
                      KILL(zNodeRef)(&z2zE1324);
                      goto end_block_exception_1826;
                    }
                  }
                  KILL(zNodeRef)(&z2zE1324);
                  KILL(zTrieBuilder)(&zintermediate);
                } else {
                  struct zNodeRef z2zE1327;
                  CREATE(zNodeRef)(&z2zE1327);
                  {
                    uint64_t z2zE1325;
                    z2zE1325 = zparent.zdepth;
                    uint64_t z2zE1326;
                    z2zE1326 = zframe.zdepth;
                    {
                      ztrie_builder_wrap_branch(&z2zE1327, zanchor, z2zE1325, z2zE1326, zchild);
                      if (have_exception) {
                        KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1316);
                        KILL(zTrieBuilder)(&z3zE1370);
                        KILL(zTrieBranchFrame)(&ztop);
                        KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE1318);
                        KILL(zTrieBuilder)(&z3zE1375);
                        KILL(zTrieBranchFrame)(&zframe);
                        KILL(zTrieBuilder)(&zpopped);
                        KILL(zNodeRef)(&zchild);
                        KILL(zTrieBuilder)(&zwith_parent);
                        KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1320);
                        KILL(zTrieBuilder)(&z3zE1376);
                        KILL(zTrieBranchFrame)(&zparent);
                        KILL(zTrieBuilder)(&z3zE1382);
                        KILL(zNodeRef)(&z2zE1327);
                        goto end_block_exception_1826;
                      }
                    }
                  }
                  {
                    ztrie_builder_attach(&z3zE1382, zpopped, zanchor, z2zE1327);
                    if (have_exception) {
                      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1316);
                      KILL(zTrieBuilder)(&z3zE1370);
                      KILL(zTrieBranchFrame)(&ztop);
                      KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE1318);
                      KILL(zTrieBuilder)(&z3zE1375);
                      KILL(zTrieBranchFrame)(&zframe);
                      KILL(zTrieBuilder)(&zpopped);
                      KILL(zNodeRef)(&zchild);
                      KILL(zTrieBuilder)(&zwith_parent);
                      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1320);
                      KILL(zTrieBuilder)(&z3zE1376);
                      KILL(zTrieBranchFrame)(&zparent);
                      KILL(zTrieBuilder)(&z3zE1382);
                      KILL(zNodeRef)(&z2zE1327);
                      goto end_block_exception_1826;
                    }
                  }
                  KILL(zNodeRef)(&z2zE1327);
                }
                goto finish_match_1821;
              }
            case_1823: ;
              {
                /* complete */
                struct zNodeRef z2zE1330;
                CREATE(zNodeRef)(&z2zE1330);
                {
                  uint64_t z2zE1328;
                  z2zE1328 = zparent.zdepth;
                  uint64_t z2zE1329;
                  z2zE1329 = zframe.zdepth;
                  {
                    ztrie_builder_wrap_branch(&z2zE1330, zanchor, z2zE1328, z2zE1329, zchild);
                    if (have_exception) {
                      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1316);
                      KILL(zTrieBuilder)(&z3zE1370);
                      KILL(zTrieBranchFrame)(&ztop);
                      KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE1318);
                      KILL(zTrieBuilder)(&z3zE1375);
                      KILL(zTrieBranchFrame)(&zframe);
                      KILL(zTrieBuilder)(&zpopped);
                      KILL(zNodeRef)(&zchild);
                      KILL(zTrieBuilder)(&zwith_parent);
                      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1320);
                      KILL(zTrieBuilder)(&z3zE1376);
                      KILL(zTrieBranchFrame)(&zparent);
                      KILL(zTrieBuilder)(&z3zE1382);
                      KILL(zNodeRef)(&z2zE1330);
                      goto end_block_exception_1826;
                    }
                  }
                }
                {
                  ztrie_builder_attach(&z3zE1382, zpopped, zanchor, z2zE1330);
                  if (have_exception) {
                    KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1316);
                    KILL(zTrieBuilder)(&z3zE1370);
                    KILL(zTrieBranchFrame)(&ztop);
                    KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE1318);
                    KILL(zTrieBuilder)(&z3zE1375);
                    KILL(zTrieBranchFrame)(&zframe);
                    KILL(zTrieBuilder)(&zpopped);
                    KILL(zNodeRef)(&zchild);
                    KILL(zTrieBuilder)(&zwith_parent);
                    KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1320);
                    KILL(zTrieBuilder)(&z3zE1376);
                    KILL(zTrieBranchFrame)(&zparent);
                    KILL(zTrieBuilder)(&z3zE1382);
                    KILL(zNodeRef)(&z2zE1330);
                    goto end_block_exception_1826;
                  }
                }
                KILL(zNodeRef)(&z2zE1330);
                goto finish_match_1821;
              }
            case_1822: ;
            finish_match_1821: ;
              COPY(zTrieBuilder)(&z3zE1376, z3zE1382);
              KILL(zTrieBuilder)(&z3zE1382);
              KILL(zTrieBranchFrame)(&zparent);
              goto finish_match_1815;
            }
          case_1820: ;
            {
              /* complete */
              struct zTrieBuilder z3zE1377;
              CREATE(zTrieBuilder)(&z3zE1377);
              {
                if (znext_common.kind != Kind_zSomezIU64zK) goto case_1819;
                uint64_t z3zE1388;
                z3zE1388 = znext_common.variants.zSomezIU64zK;
                struct zTrieBuilder z3zE1389;
                CREATE(zTrieBuilder)(&z3zE1389);
                ztrie_builder_push(&z3zE1389, zpopped, z3zE1388);
                struct zNodeRef z2zE1332;
                CREATE(zNodeRef)(&z2zE1332);
                {
                  uint64_t z2zE1331;
                  z2zE1331 = zframe.zdepth;
                  {
                    ztrie_builder_wrap_branch(&z2zE1332, zanchor, z3zE1388, z2zE1331, zchild);
                    if (have_exception) {
                      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1316);
                      KILL(zTrieBuilder)(&z3zE1370);
                      KILL(zTrieBranchFrame)(&ztop);
                      KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE1318);
                      KILL(zTrieBuilder)(&z3zE1375);
                      KILL(zTrieBranchFrame)(&zframe);
                      KILL(zTrieBuilder)(&zpopped);
                      KILL(zNodeRef)(&zchild);
                      KILL(zTrieBuilder)(&zwith_parent);
                      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1320);
                      KILL(zTrieBuilder)(&z3zE1376);
                      KILL(zTrieBuilder)(&z3zE1377);
                      KILL(zTrieBuilder)(&z3zE1389);
                      KILL(zNodeRef)(&z2zE1332);
                      goto end_block_exception_1826;
                    }
                  }
                }
                {
                  ztrie_builder_attach(&z3zE1377, z3zE1389, zanchor, z2zE1332);
                  if (have_exception) {
                    KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1316);
                    KILL(zTrieBuilder)(&z3zE1370);
                    KILL(zTrieBranchFrame)(&ztop);
                    KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE1318);
                    KILL(zTrieBuilder)(&z3zE1375);
                    KILL(zTrieBranchFrame)(&zframe);
                    KILL(zTrieBuilder)(&zpopped);
                    KILL(zNodeRef)(&zchild);
                    KILL(zTrieBuilder)(&zwith_parent);
                    KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1320);
                    KILL(zTrieBuilder)(&z3zE1376);
                    KILL(zTrieBuilder)(&z3zE1377);
                    KILL(zTrieBuilder)(&z3zE1389);
                    KILL(zNodeRef)(&z2zE1332);
                    goto end_block_exception_1826;
                  }
                }
                KILL(zNodeRef)(&z2zE1332);
                KILL(zTrieBuilder)(&z3zE1389);
                goto finish_match_1817;
              }
            case_1819: ;
              {
                /* complete */
                uint64_t z3zE1390;
                z3zE1390 = zframe.zdepth;
                struct zNodeRef zroot;
                CREATE(zNodeRef)(&zroot);
                {
                  bool z2zE1334;
                  z2zE1334 = (z3zE1390 == UINT64_C(0));
                  if (z2zE1334) {  COPY(zNodeRef)(&zroot, zchild);  } else {
                    struct zTriePath z2zE1335;
                    {
                      z2zE1335 = zpath_take(zanchor, z3zE1390);
                      if (have_exception) {
                        KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1316);
                        KILL(zTrieBuilder)(&z3zE1370);
                        KILL(zTrieBranchFrame)(&ztop);
                        KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE1318);
                        KILL(zTrieBuilder)(&z3zE1375);
                        KILL(zTrieBranchFrame)(&zframe);
                        KILL(zTrieBuilder)(&zpopped);
                        KILL(zNodeRef)(&zchild);
                        KILL(zTrieBuilder)(&zwith_parent);
                        KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1320);
                        KILL(zTrieBuilder)(&z3zE1376);
                        KILL(zTrieBuilder)(&z3zE1377);
                        KILL(zNodeRef)(&zroot);
                        goto end_block_exception_1826;
                      }
                    }
                    {
                      zextension_child_ref(&zroot, z2zE1335, zchild);
                      if (have_exception) {
                        KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1316);
                        KILL(zTrieBuilder)(&z3zE1370);
                        KILL(zTrieBranchFrame)(&ztop);
                        KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE1318);
                        KILL(zTrieBuilder)(&z3zE1375);
                        KILL(zTrieBranchFrame)(&zframe);
                        KILL(zTrieBuilder)(&zpopped);
                        KILL(zNodeRef)(&zchild);
                        KILL(zTrieBuilder)(&zwith_parent);
                        KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1320);
                        KILL(zTrieBuilder)(&z3zE1376);
                        KILL(zTrieBuilder)(&z3zE1377);
                        KILL(zNodeRef)(&zroot);
                        goto end_block_exception_1826;
                      }
                    }
                  }
                }
                zz5listz8z5structz0zzTrieBranchFramez9 z2zE1333;
                CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1333);
                COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1333, zpopped.zframes);
                z3zE1377.zcomplete = true;
                COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&((&z3zE1377)->zframes), z2zE1333);
                COPY(zNodeRef)(&((&z3zE1377)->zroot), zroot);
                KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1333);
                KILL(zNodeRef)(&zroot);
                goto finish_match_1817;
              }
            case_1818: ;
            finish_match_1817: ;
              COPY(zTrieBuilder)(&z3zE1376, z3zE1377);
              KILL(zTrieBuilder)(&z3zE1377);
              goto finish_match_1815;
            }
          case_1816: ;
          finish_match_1815: ;
            COPY(zTrieBuilder)(&zwith_parent, z3zE1376);
            KILL(zTrieBuilder)(&z3zE1376);
            KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1320);
          }
          uint64_t z2zE1319;
          {    z2zE1319 = (zfuel - UINT64_C(1));
          }
          {
            ztrie_builder_close(&z3zE1375, zwith_parent, zanchor, znext_common, z2zE1319);
            if (have_exception) {
              KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1316);
              KILL(zTrieBuilder)(&z3zE1370);
              KILL(zTrieBranchFrame)(&ztop);
              KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE1318);
              KILL(zTrieBuilder)(&z3zE1375);
              KILL(zTrieBranchFrame)(&zframe);
              KILL(zTrieBuilder)(&zpopped);
              KILL(zNodeRef)(&zchild);
              KILL(zTrieBuilder)(&zwith_parent);
              goto end_block_exception_1826;
            }
          }
          KILL(zTrieBuilder)(&zwith_parent);
          KILL(zNodeRef)(&zchild);
          KILL(zTrieBuilder)(&zpopped);
          KILL(zTrieBranchFrame)(&zframe);
          goto finish_match_1813;
        }
      case_1814: ;
        sail_match_failure("trie_builder_close");
      finish_match_1813: ;
        COPY(zTrieBuilder)(&z3zE1370, z3zE1375);
        KILL(zTrieBuilder)(&z3zE1375);
        KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE1318);
      }
      KILL(zTrieBranchFrame)(&ztop);
      goto finish_match_1808;
    }
  case_1809: ;
  finish_match_1808: ;
    COPY(zTrieBuilder)((*(&z8zE534)), z3zE1370);
    KILL(zTrieBuilder)(&z3zE1370);
    KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1316);
  }
end_function_1825: ;
  goto end_function_4012;
end_block_exception_1826: ;
  goto end_function_4012;
end_function_4012: ;
}

void ztrie_item_next_common(struct zoptionzIU64zK *z8zE535, struct zTrieItem zitem, struct zoptionzIRTriePathzK znext_key)
{
  struct zoptionzIU64zK z3zE1364;
  CREATE(zoptionzIU64zK)(&z3zE1364);
  {
    if (znext_key.kind != Kind_zNonezIRTriePathzK) goto case_1805;
    zNonezIU64zK(&z3zE1364, UNIT);
    goto finish_match_1803;
  }
case_1805: ;
  {
    /* complete */
    struct zTriePath znext;
    znext = znext_key.variants.zSomezIRTriePathzK;
    uint64_t zcommon;
    {
      struct zTriePath z2zE1313;
      z2zE1313 = zitem.zpath;
      {
        zcommon = zcommon_prefix_from(z2zE1313, znext, UINT64_C(0));
        if (have_exception) {
          KILL(zoptionzIU64zK)(&z3zE1364);
          goto end_block_exception_1807;
        }
      }
    }
    bool z2zE1310;
    {
      bool z2zE1309;
      {
        bool z2zE1304;
        {
          struct zTriePath z2zE1303;
          z2zE1303 = zitem.zpath;
          z2zE1304 = zpath_lt(z2zE1303, znext);
        }
        z2zE1309 = not(z2zE1304);
      }
      bool z3zE1366;
      if (z2zE1309) {  z3zE1366 = true;  } else {
        bool z2zE1308;
        {
          uint64_t z2zE1306;
          {
            struct zTriePath z2zE1305;
            z2zE1305 = zitem.zpath;
            z2zE1306 = zpath_len(z2zE1305);
          }
          z2zE1308 = (!(zcommon < z2zE1306));
        }
        bool z3zE1365;
        if (z2zE1308) {  z3zE1365 = true;  } else {
          uint64_t z2zE1307;
          z2zE1307 = zpath_len(znext);
          z3zE1365 = (!(zcommon < z2zE1307));
        }
        z3zE1366 = z3zE1365;
      }
      z2zE1310 = z3zE1366;
    }
    unit z3zE1367;
    if (z2zE1310) {
      struct zexception z2zE1311;
      CREATE(zexception)(&z2zE1311);
      zInvalidBlock(&z2zE1311, zWitnessDeficient);
      COPY(zexception)(current_exception, z2zE1311);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/lib/mpt/updates.sail:305.16-305.52");
      KILL(zoptionzIU64zK)(&z3zE1364);
      KILL(zexception)(&z2zE1311);
      goto end_block_exception_1807;
      /* unreachable after throw */
      KILL(zexception)(&z2zE1311);
    } else {  z3zE1367 = UNIT;  }
    uint64_t z2zE1312;
    {
      z2zE1312 = zto_trie_depth(zcommon);
      if (have_exception) {
        KILL(zoptionzIU64zK)(&z3zE1364);
        goto end_block_exception_1807;
      }
    }
    zSomezIU64zK(&z3zE1364, z2zE1312);
    goto finish_match_1803;
  }
case_1804: ;
finish_match_1803: ;
  COPY(zoptionzIU64zK)((*(&z8zE535)), z3zE1364);
  KILL(zoptionzIU64zK)(&z3zE1364);
end_function_1806: ;
  goto end_function_4011;
end_block_exception_1807: ;
  goto end_function_4011;
end_function_4011: ;
}

void ztrie_insert_item(struct zTrieBuilder *z8zE536, struct zTrieBuilder zbuilder, struct zTrieItem zitem, struct zoptionzIRTriePathzK znext_key)
{
  bool z2zE1287;
  z2zE1287 = zbuilder.zcomplete;
  unit z3zE1348;
  if (z2zE1287) {
    struct zexception z2zE1288;
    CREATE(zexception)(&z2zE1288);
    zInvalidBlock(&z2zE1288, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE1288);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/updates.sail:315.8-315.44");
    KILL(zexception)(&z2zE1288);
    goto end_block_exception_1802;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1288);
  } else {  z3zE1348 = UNIT;  }
  struct zoptionzIU64zK znext_common;
  CREATE(zoptionzIU64zK)(&znext_common);
  {
    ztrie_item_next_common(&znext_common, zitem, znext_key);
    if (have_exception) {
      KILL(zoptionzIU64zK)(&znext_common);
      goto end_block_exception_1802;
    }
  }
  bool zopen_child;
  {
    bool z3zE1349;
    {
      if (znext_common.kind != Kind_zNonezIU64zK) goto case_1794;
      z3zE1349 = false;
      goto finish_match_1789;
    }
  case_1794: ;
    {
      /* complete */
      uint64_t zcommon;
      zcommon = znext_common.variants.zSomezIU64zK;
      zz5listz8z5structz0zzTrieBranchFramez9 z2zE1301;
      CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1301);
      COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1301, zbuilder.zframes);
      bool z3zE1350;
      {
        if (!((z2zE1301 == NULL))) goto case_1793;
        z3zE1350 = true;
        goto finish_match_1791;
      }
    case_1793: ;
      {
        /* complete */
        struct zTrieBranchFrame ztop;
        CREATE(zTrieBranchFrame)(&ztop);
        COPY(zTrieBranchFrame)(&ztop, (*z2zE1301).hd);
        uint64_t z2zE1302;
        z2zE1302 = ztop.zdepth;
        z3zE1350 = (z2zE1302 < zcommon);
        KILL(zTrieBranchFrame)(&ztop);
        goto finish_match_1791;
      }
    case_1792: ;
    finish_match_1791: ;
      z3zE1349 = z3zE1350;
      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1301);
      goto finish_match_1789;
    }
  case_1790: ;
  finish_match_1789: ;
    zopen_child = z3zE1349;
  }
  if (zopen_child) {
    struct zTrieBuilder z3zE1359;
    CREATE(zTrieBuilder)(&z3zE1359);
    {
      if (znext_common.kind != Kind_zSomezIU64zK) goto case_1800;
      uint64_t z3zE1362;
      z3zE1362 = znext_common.variants.zSomezIU64zK;
      struct zTrieBuilder zopened;
      CREATE(zTrieBuilder)(&zopened);
      ztrie_builder_push(&zopened, zbuilder, z3zE1362);
      struct zTriePath z2zE1290;
      z2zE1290 = zitem.zpath;
      struct zNodeRef z2zE1291;
      CREATE(zNodeRef)(&z2zE1291);
      {
        uint64_t z2zE1289;
        {    z2zE1289 = (z3zE1362 + UINT64_C(1));
        }
        {
          zitem_ref(&z2zE1291, zitem, z2zE1289);
          if (have_exception) {
            KILL(zoptionzIU64zK)(&znext_common);
            KILL(zTrieBuilder)(&z3zE1359);
            KILL(zTrieBuilder)(&zopened);
            KILL(zNodeRef)(&z2zE1291);
            goto end_block_exception_1802;
          }
        }
      }
      {
        ztrie_builder_attach(&z3zE1359, zopened, z2zE1290, z2zE1291);
        if (have_exception) {
          KILL(zoptionzIU64zK)(&znext_common);
          KILL(zTrieBuilder)(&z3zE1359);
          KILL(zTrieBuilder)(&zopened);
          KILL(zNodeRef)(&z2zE1291);
          goto end_block_exception_1802;
        }
      }
      KILL(zNodeRef)(&z2zE1291);
      KILL(zTrieBuilder)(&zopened);
      goto finish_match_1798;
    }
  case_1800: ;
    {
      /* complete */
      struct zexception z2zE1292;
      CREATE(zexception)(&z2zE1292);
      zInvalidBlock(&z2zE1292, zWitnessDeficient);
      COPY(zexception)(current_exception, z2zE1292);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/lib/mpt/updates.sail:331.22-331.58");
      KILL(zoptionzIU64zK)(&znext_common);
      KILL(zTrieBuilder)(&z3zE1359);
      KILL(zexception)(&z2zE1292);
      goto end_block_exception_1802;
      /* unreachable after throw */
      KILL(zexception)(&z2zE1292);
      goto finish_match_1798;
    }
  case_1799: ;
  finish_match_1798: ;
    COPY(zTrieBuilder)((*(&z8zE536)), z3zE1359);
    KILL(zTrieBuilder)(&z3zE1359);
  } else {
    zz5listz8z5structz0zzTrieBranchFramez9 z2zE1293;
    CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1293);
    COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1293, zbuilder.zframes);
    struct zTrieBuilder z3zE1355;
    CREATE(zTrieBuilder)(&z3zE1355);
    {
      if (!((z2zE1293 == NULL))) goto case_1797;
      zz5listz8z5structz0zzTrieBranchFramez9 z2zE1294;
      CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1294);
      COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1294, zbuilder.zframes);
      struct zNodeRef z2zE1295;
      CREATE(zNodeRef)(&z2zE1295);
      {
        zitem_ref(&z2zE1295, zitem, UINT64_C(0));
        if (have_exception) {
          KILL(zoptionzIU64zK)(&znext_common);
          KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1293);
          KILL(zTrieBuilder)(&z3zE1355);
          KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1294);
          KILL(zNodeRef)(&z2zE1295);
          goto end_block_exception_1802;
        }
      }
      z3zE1355.zcomplete = true;
      COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&((&z3zE1355)->zframes), z2zE1294);
      COPY(zNodeRef)(&((&z3zE1355)->zroot), z2zE1295);
      KILL(zNodeRef)(&z2zE1295);
      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1294);
      goto finish_match_1795;
    }
  case_1797: ;
    {
      /* complete */
      struct zTrieBranchFrame z3zE1363;
      CREATE(zTrieBranchFrame)(&z3zE1363);
      COPY(zTrieBranchFrame)(&z3zE1363, (*z2zE1293).hd);
      struct zTrieBuilder zattached;
      CREATE(zTrieBuilder)(&zattached);
      {
        struct zTriePath z2zE1299;
        z2zE1299 = zitem.zpath;
        struct zNodeRef z2zE1300;
        CREATE(zNodeRef)(&z2zE1300);
        {
          uint64_t z2zE1298;
          {
            uint64_t z2zE1297;
            z2zE1297 = z3zE1363.zdepth;
            {    z2zE1298 = (z2zE1297 + UINT64_C(1));
            }
          }
          {
            zitem_ref(&z2zE1300, zitem, z2zE1298);
            if (have_exception) {
              KILL(zoptionzIU64zK)(&znext_common);
              KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1293);
              KILL(zTrieBuilder)(&z3zE1355);
              KILL(zTrieBranchFrame)(&z3zE1363);
              KILL(zTrieBuilder)(&zattached);
              KILL(zNodeRef)(&z2zE1300);
              goto end_block_exception_1802;
            }
          }
        }
        {
          ztrie_builder_attach(&zattached, zbuilder, z2zE1299, z2zE1300);
          if (have_exception) {
            KILL(zoptionzIU64zK)(&znext_common);
            KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1293);
            KILL(zTrieBuilder)(&z3zE1355);
            KILL(zTrieBranchFrame)(&z3zE1363);
            KILL(zTrieBuilder)(&zattached);
            KILL(zNodeRef)(&z2zE1300);
            goto end_block_exception_1802;
          }
        }
        KILL(zNodeRef)(&z2zE1300);
      }
      struct zTriePath z2zE1296;
      z2zE1296 = zitem.zpath;
      {
        ztrie_builder_close(&z3zE1355, zattached, z2zE1296, znext_common, UINT64_C(64));
        if (have_exception) {
          KILL(zoptionzIU64zK)(&znext_common);
          KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1293);
          KILL(zTrieBuilder)(&z3zE1355);
          KILL(zTrieBranchFrame)(&z3zE1363);
          KILL(zTrieBuilder)(&zattached);
          goto end_block_exception_1802;
        }
      }
      KILL(zTrieBuilder)(&zattached);
      KILL(zTrieBranchFrame)(&z3zE1363);
      goto finish_match_1795;
    }
  case_1796: ;
  finish_match_1795: ;
    COPY(zTrieBuilder)((*(&z8zE536)), z3zE1355);
    KILL(zTrieBuilder)(&z3zE1355);
    KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1293);
  }
  KILL(zoptionzIU64zK)(&znext_common);
end_function_1801: ;
  goto end_function_4010;
end_block_exception_1802: ;
  goto end_function_4010;
end_function_4010: ;
}

void ztrie_sink_empty(struct zTrieItemSink *z8zE537, unit z3zE1346)
{
  struct zTrieBuilder z2zE1285;
  CREATE(zTrieBuilder)(&z2zE1285);
  ztrie_builder_empty(&z2zE1285, UNIT);
  struct zoptionzIRTrieItemzK z2zE1286;
  CREATE(zoptionzIRTrieItemzK)(&z2zE1286);
  zNonezIRTrieItemzK(&z2zE1286, UNIT);
  struct zTrieItemSink z3zE1347;
  CREATE(zTrieItemSink)(&z3zE1347);
  COPY(zTrieBuilder)(&((&z3zE1347)->zbuilder), z2zE1285);
  COPY(zoptionzIRTrieItemzK)(&((&z3zE1347)->zpending), z2zE1286);
  COPY(zTrieItemSink)((*(&z8zE537)), z3zE1347);
  KILL(zTrieItemSink)(&z3zE1347);
  KILL(zoptionzIRTrieItemzK)(&z2zE1286);
  KILL(zTrieBuilder)(&z2zE1285);
end_function_1787: ;
  goto end_function_4009;
end_block_exception_1788: ;
  goto end_function_4009;
end_function_4009: ;
}

void ztrie_sink_emit(struct zTrieItemSink *z8zE538, struct zTrieItemSink zsink, struct zTrieItem zitem)
{
  struct zoptionzIRTrieItemzK z2zE1277;
  CREATE(zoptionzIRTrieItemzK)(&z2zE1277);
  COPY(zoptionzIRTrieItemzK)(&z2zE1277, zsink.zpending);
  struct zTrieItemSink z3zE1341;
  CREATE(zTrieItemSink)(&z3zE1341);
  {
    if (z2zE1277.kind != Kind_zNonezIRTrieItemzK) goto case_1784;
    struct zTrieBuilder z2zE1278;
    CREATE(zTrieBuilder)(&z2zE1278);
    COPY(zTrieBuilder)(&z2zE1278, zsink.zbuilder);
    struct zoptionzIRTrieItemzK z2zE1279;
    CREATE(zoptionzIRTrieItemzK)(&z2zE1279);
    zSomezIRTrieItemzK(&z2zE1279, zitem);
    COPY(zTrieBuilder)(&((&z3zE1341)->zbuilder), z2zE1278);
    COPY(zoptionzIRTrieItemzK)(&((&z3zE1341)->zpending), z2zE1279);
    KILL(zoptionzIRTrieItemzK)(&z2zE1279);
    KILL(zTrieBuilder)(&z2zE1278);
    goto finish_match_1782;
  }
case_1784: ;
  {
    /* complete */
    struct zTrieItem zprevious;
    CREATE(zTrieItem)(&zprevious);
    COPY(zTrieItem)(&zprevious, z2zE1277.variants.zSomezIRTrieItemzK);
    struct zTrieBuilder z2zE1283;
    CREATE(zTrieBuilder)(&z2zE1283);
    {
      struct zTrieBuilder z2zE1281;
      CREATE(zTrieBuilder)(&z2zE1281);
      COPY(zTrieBuilder)(&z2zE1281, zsink.zbuilder);
      struct zoptionzIRTriePathzK z2zE1282;
      CREATE(zoptionzIRTriePathzK)(&z2zE1282);
      {
        struct zTriePath z2zE1280;
        z2zE1280 = zitem.zpath;
        zSomezIRTriePathzK(&z2zE1282, z2zE1280);
      }
      {
        ztrie_insert_item(&z2zE1283, z2zE1281, zprevious, z2zE1282);
        if (have_exception) {
          KILL(zoptionzIRTrieItemzK)(&z2zE1277);
          KILL(zTrieItemSink)(&z3zE1341);
          KILL(zTrieItem)(&zprevious);
          KILL(zTrieBuilder)(&z2zE1283);
          KILL(zTrieBuilder)(&z2zE1281);
          KILL(zoptionzIRTriePathzK)(&z2zE1282);
          goto end_block_exception_1786;
        }
      }
      KILL(zoptionzIRTriePathzK)(&z2zE1282);
      KILL(zTrieBuilder)(&z2zE1281);
    }
    struct zoptionzIRTrieItemzK z2zE1284;
    CREATE(zoptionzIRTrieItemzK)(&z2zE1284);
    zSomezIRTrieItemzK(&z2zE1284, zitem);
    COPY(zTrieBuilder)(&((&z3zE1341)->zbuilder), z2zE1283);
    COPY(zoptionzIRTrieItemzK)(&((&z3zE1341)->zpending), z2zE1284);
    KILL(zoptionzIRTrieItemzK)(&z2zE1284);
    KILL(zTrieBuilder)(&z2zE1283);
    KILL(zTrieItem)(&zprevious);
    goto finish_match_1782;
  }
case_1783: ;
finish_match_1782: ;
  COPY(zTrieItemSink)((*(&z8zE538)), z3zE1341);
  KILL(zTrieItemSink)(&z3zE1341);
  KILL(zoptionzIRTrieItemzK)(&z2zE1277);
end_function_1785: ;
  goto end_function_4008;
end_block_exception_1786: ;
  goto end_function_4008;
end_function_4008: ;
}

void ztrie_sink_finish(struct zTrieItemSink *z8zE539, struct zTrieItemSink zsink)
{
  struct zoptionzIRTrieItemzK z2zE1272;
  CREATE(zoptionzIRTrieItemzK)(&z2zE1272);
  COPY(zoptionzIRTrieItemzK)(&z2zE1272, zsink.zpending);
  struct zTrieItemSink z3zE1337;
  CREATE(zTrieItemSink)(&z3zE1337);
  {
    if (z2zE1272.kind != Kind_zNonezIRTrieItemzK) goto case_1779;
    COPY(zTrieItemSink)(&z3zE1337, zsink);
    goto finish_match_1777;
  }
case_1779: ;
  {
    /* complete */
    struct zTrieItem zitem;
    CREATE(zTrieItem)(&zitem);
    COPY(zTrieItem)(&zitem, z2zE1272.variants.zSomezIRTrieItemzK);
    struct zTrieBuilder z2zE1275;
    CREATE(zTrieBuilder)(&z2zE1275);
    {
      struct zTrieBuilder z2zE1273;
      CREATE(zTrieBuilder)(&z2zE1273);
      COPY(zTrieBuilder)(&z2zE1273, zsink.zbuilder);
      struct zoptionzIRTriePathzK z2zE1274;
      CREATE(zoptionzIRTriePathzK)(&z2zE1274);
      zNonezIRTriePathzK(&z2zE1274, UNIT);
      {
        ztrie_insert_item(&z2zE1275, z2zE1273, zitem, z2zE1274);
        if (have_exception) {
          KILL(zoptionzIRTrieItemzK)(&z2zE1272);
          KILL(zTrieItemSink)(&z3zE1337);
          KILL(zTrieItem)(&zitem);
          KILL(zTrieBuilder)(&z2zE1275);
          KILL(zTrieBuilder)(&z2zE1273);
          KILL(zoptionzIRTriePathzK)(&z2zE1274);
          goto end_block_exception_1781;
        }
      }
      KILL(zoptionzIRTriePathzK)(&z2zE1274);
      KILL(zTrieBuilder)(&z2zE1273);
    }
    struct zoptionzIRTrieItemzK z2zE1276;
    CREATE(zoptionzIRTrieItemzK)(&z2zE1276);
    zNonezIRTrieItemzK(&z2zE1276, UNIT);
    COPY(zTrieBuilder)(&((&z3zE1337)->zbuilder), z2zE1275);
    COPY(zoptionzIRTrieItemzK)(&((&z3zE1337)->zpending), z2zE1276);
    KILL(zoptionzIRTrieItemzK)(&z2zE1276);
    KILL(zTrieBuilder)(&z2zE1275);
    KILL(zTrieItem)(&zitem);
    goto finish_match_1777;
  }
case_1778: ;
finish_match_1777: ;
  COPY(zTrieItemSink)((*(&z8zE539)), z3zE1337);
  KILL(zTrieItemSink)(&z3zE1337);
  KILL(zoptionzIRTrieItemzK)(&z2zE1272);
end_function_1780: ;
  goto end_function_4007;
end_block_exception_1781: ;
  goto end_function_4007;
end_function_4007: ;
}

sail_fixed_bytes_32 ztrie_builder_root(struct zTrieBuilder zbuilder)
{
  sail_fixed_bytes_32 z8zE540;
  zz5listz8z5structz0zzTrieBranchFramez9 z2zE1268;
  CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1268);
  COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1268, zbuilder.zframes);
  sail_fixed_bytes_32 z3zE1334;
  {
    if ((z2zE1268 == NULL)) goto case_1774;
    struct zexception z2zE1269;
    CREATE(zexception)(&z2zE1269);
    zInvalidBlock(&z2zE1269, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE1269);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/updates.sail:376.18-376.54");
    KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1268);
    KILL(zexception)(&z2zE1269);
    goto end_block_exception_1776;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1269);
    goto finish_match_1772;
  }
case_1774: ;
  {
    /* complete */
    bool z2zE1270;
    z2zE1270 = zbuilder.zcomplete;
    if (z2zE1270) {
      struct zNodeRef z2zE1271;
      CREATE(zNodeRef)(&z2zE1271);
      COPY(zNodeRef)(&z2zE1271, zbuilder.zroot);
      z3zE1334 = ztrie_ref_to_root(z2zE1271);
      KILL(zNodeRef)(&z2zE1271);
    } else {  z3zE1334 = zEMPTY_TRIE_ROOT;  }
    goto finish_match_1772;
  }
case_1773: ;
finish_match_1772: ;
  z8zE540 = z3zE1334;
  KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE1268);
end_function_1775: ;
  return z8zE540;
end_block_exception_1776: ;

  return fixed_bytes_32_zero();
}

sail_fixed_bytes_32 ztrie_sink_root(struct zTrieItemSink zsink)
{
  sail_fixed_bytes_32 z8zE541;
  struct zoptionzIRTrieItemzK z2zE1265;
  CREATE(zoptionzIRTrieItemzK)(&z2zE1265);
  COPY(zoptionzIRTrieItemzK)(&z2zE1265, zsink.zpending);
  sail_fixed_bytes_32 z3zE1331;
  {
    if (z2zE1265.kind != Kind_zSomezIRTrieItemzK) goto case_1769;
    struct zexception z2zE1266;
    CREATE(zexception)(&z2zE1266);
    zInvalidBlock(&z2zE1266, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE1266);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/updates.sail:387.19-387.55");
    KILL(zoptionzIRTrieItemzK)(&z2zE1265);
    KILL(zexception)(&z2zE1266);
    goto end_block_exception_1771;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1266);
    goto finish_match_1767;
  }
case_1769: ;
  {
    /* complete */
    struct zTrieBuilder z2zE1267;
    CREATE(zTrieBuilder)(&z2zE1267);
    COPY(zTrieBuilder)(&z2zE1267, zsink.zbuilder);
    {
      z3zE1331 = ztrie_builder_root(z2zE1267);
      if (have_exception) {
        KILL(zoptionzIRTrieItemzK)(&z2zE1265);
        KILL(zTrieBuilder)(&z2zE1267);
        goto end_block_exception_1771;
      }
    }
    KILL(zTrieBuilder)(&z2zE1267);
    goto finish_match_1767;
  }
case_1768: ;
finish_match_1767: ;
  z8zE541 = z3zE1331;
  KILL(zoptionzIRTrieItemzK)(&z2zE1265);
end_function_1770: ;
  return z8zE541;
end_block_exception_1771: ;

  return fixed_bytes_32_zero();
}

