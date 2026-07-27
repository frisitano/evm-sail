/* Generated from sail/lib/mpt/updates.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
void zcached_account_trie_update_next(struct zoptionzIRTrieUpdatezK *z8zE564, unit z3zE1135)
{
  zNonezIRTrieUpdatezK((*(&z8zE564)), UNIT);
end_function_1659: ;
  goto end_function_3618;
end_block_exception_1660: ;
  goto end_function_3618;
end_function_3618: ;
}

struct zByteSliceFields zencode_state_account(struct zAccountInfo zinfo, sail_fixed_bytes_32 zstorage_root)
{
  struct zByteSliceFields z8zE565;
  uint64_t znonce_length;
  {
    uint64_t z2zE1007;
    z2zE1007 = zinfo.znonce;
    znonce_length = zrlp_uint_word_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE1007);
  }
  uint64_t zbalance_length;
  {
    sail_u256 z2zE1006;
    z2zE1006 = zinfo.zbalance;
    zbalance_length = zrlp_uint_word_sizze(z2zE1006);
  }
  uint64_t zcontent_len;
  {
    uint64_t z2zE1005;
    {
      uint64_t z2zE1004;
      {    z2zE1004 = (znonce_length + zbalance_length);
      }
      {    z2zE1005 = (z2zE1004 + UINT64_C(33));
      }
    }
    {    zcontent_len = (z2zE1005 + UINT64_C(33));
    }
  }
  uint64_t zstart;
  zstart = zscratch_begin(UNIT);
  unit z3zE1134;
  z3zE1134 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zcontent_len);
  uint64_t z2zE999;
  z2zE999 = zinfo.znonce;
  unit z3zE1133;
  z3zE1133 = zrlp_write_uint_wordzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE999);
  sail_u256 z2zE1000;
  z2zE1000 = zinfo.zbalance;
  unit z3zE1132;
  z3zE1132 = zrlp_write_uint_word(z2zE1000);
  sail_u256 z2zE1001;
  z2zE1001 = evmsail_hash_to_word(zstorage_root);
  unit z3zE1131;
  z3zE1131 = zrlp_write_word(z2zE1001);
  sail_u256 z2zE1003;
  {
    sail_fixed_bytes_32 z2zE1002;
    z2zE1002 = zinfo.zcode_hash;
    z2zE1003 = evmsail_hash_to_word(z2zE1002);
  }
  unit z3zE1130;
  z3zE1130 = zrlp_write_word(z2zE1003);
  z8zE565 = zrlp_finish(zstart);
end_function_1657: ;
  return z8zE565;
end_block_exception_1658: ;
  struct zByteSliceFields z8zE917 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE917;
}

void zaccount_update(struct zTrieUpdate *z8zE566, struct zAcctTrieEntry ztrie_entry, sail_fixed_bytes_32 zstorage_root)
{
  struct zAcctEntry zentry;
  zentry = ztrie_entry.zentry;
  struct zAccount zcurrent;
  {
    struct zAcctValue z2zE998;
    z2zE998 = zentry.zvalue;
    zcurrent = z2zE998.zcurr;
  }
  struct zTriePath zkey;
  {
    sail_fixed_bytes_32 z2zE997;
    z2zE997 = ztrie_entry.zaddress_hash;
    zkey = zpath_from_hash(z2zE997);
  }
  bool z2zE992;
  {
    bool z2zE991;
    {
      bool z2zE989;
      z2zE989 = zcurrent.zpresent;
      z2zE991 = not(z2zE989);
    }
    bool z3zE1127;
    if (z2zE991) {  z3zE1127 = true;  } else {
      struct zAccountInfo z2zE990;
      z2zE990 = zcurrent.zinfo;
      z3zE1127 = zaccount_info_empty(z2zE990);
    }
    z2zE992 = z3zE1127;
  }
  if (z2zE992) {
    struct zTrieChange z2zE993;
    CREATE(zTrieChange)(&z2zE993);
    zTrieDelete(&z2zE993, UNIT);
    struct zTrieUpdate z3zE1129;
    CREATE(zTrieUpdate)(&z3zE1129);
    z3zE1129.zchange = z2zE993;
    z3zE1129.zkey = zkey;
    (*(z8zE566)) = z3zE1129;
    KILL(zTrieUpdate)(&z3zE1129);
    KILL(zTrieChange)(&z2zE993);
  } else {
    struct zTrieChange z2zE996;
    CREATE(zTrieChange)(&z2zE996);
    {
      struct zByteSliceFields z2zE995;
      {
        struct zAccountInfo z2zE994;
        z2zE994 = zcurrent.zinfo;
        z2zE995 = zencode_state_account(z2zE994, zstorage_root);
      }
      zTriePut(&z2zE996, z2zE995);
    }
    struct zTrieUpdate z3zE1128;
    CREATE(zTrieUpdate)(&z3zE1128);
    z3zE1128.zchange = z2zE996;
    z3zE1128.zkey = zkey;
    (*(z8zE566)) = z3zE1128;
    KILL(zTrieUpdate)(&z3zE1128);
    KILL(zTrieChange)(&z2zE996);
  }
end_function_1655: ;
  goto end_function_3617;
end_block_exception_1656: ;
  goto end_function_3617;
end_function_3617: ;
}

bool zaccount_value_changed(struct zAcctValue zvalue)
{
  bool z8zE567;
  bool z2zE988;
  {
    bool z2zE952;
    {
      uint64_t z2zE950;
      {
        struct zAccountInfo z2zE947;
        {
          struct zAccount z2zE946;
          z2zE946 = zvalue.zcurr;
          z2zE947 = z2zE946.zinfo;
        }
        z2zE950 = z2zE947.znonce;
      }
      uint64_t z2zE951;
      {
        struct zAccountInfo z2zE949;
        {
          struct zAccount z2zE948;
          z2zE948 = zvalue.zorig;
          z2zE949 = z2zE948.zinfo;
        }
        z2zE951 = z2zE949.znonce;
      }
      z2zE952 = (z2zE950 == z2zE951);
    }
    z2zE988 = not(z2zE952);
  }
  bool z3zE1126;
  if (z2zE988) {  z3zE1126 = true;  } else {
    bool z2zE987;
    {
      bool z2zE959;
      {
        sail_u256 z2zE957;
        {
          struct zAccountInfo z2zE954;
          {
            struct zAccount z2zE953;
            z2zE953 = zvalue.zcurr;
            z2zE954 = z2zE953.zinfo;
          }
          z2zE957 = z2zE954.zbalance;
        }
        sail_u256 z2zE958;
        {
          struct zAccountInfo z2zE956;
          {
            struct zAccount z2zE955;
            z2zE955 = zvalue.zorig;
            z2zE956 = z2zE955.zinfo;
          }
          z2zE958 = z2zE956.zbalance;
        }
        z2zE959 = eq_u256(z2zE957, z2zE958);
      }
      z2zE987 = not(z2zE959);
    }
    bool z3zE1125;
    if (z2zE987) {  z3zE1125 = true;  } else {
      bool z2zE986;
      {
        bool z2zE966;
        {
          sail_fixed_bytes_32 z2zE964;
          {
            struct zAccountInfo z2zE961;
            {
              struct zAccount z2zE960;
              z2zE960 = zvalue.zcurr;
              z2zE961 = z2zE960.zinfo;
            }
            z2zE964 = z2zE961.zstorage_root;
          }
          sail_fixed_bytes_32 z2zE965;
          {
            struct zAccountInfo z2zE963;
            {
              struct zAccount z2zE962;
              z2zE962 = zvalue.zorig;
              z2zE963 = z2zE962.zinfo;
            }
            z2zE965 = z2zE963.zstorage_root;
          }
          z2zE966 = eq_fixed_bytes_32(z2zE964, z2zE965);
        }
        z2zE986 = not(z2zE966);
      }
      bool z3zE1124;
      if (z2zE986) {  z3zE1124 = true;  } else {
        bool z2zE985;
        {
          bool z2zE973;
          {
            sail_fixed_bytes_32 z2zE971;
            {
              struct zAccountInfo z2zE968;
              {
                struct zAccount z2zE967;
                z2zE967 = zvalue.zcurr;
                z2zE968 = z2zE967.zinfo;
              }
              z2zE971 = z2zE968.zcode_hash;
            }
            sail_fixed_bytes_32 z2zE972;
            {
              struct zAccountInfo z2zE970;
              {
                struct zAccount z2zE969;
                z2zE969 = zvalue.zorig;
                z2zE970 = z2zE969.zinfo;
              }
              z2zE972 = z2zE970.zcode_hash;
            }
            z2zE973 = eq_fixed_bytes_32(z2zE971, z2zE972);
          }
          z2zE985 = not(z2zE973);
        }
        bool z3zE1123;
        if (z2zE985) {  z3zE1123 = true;  } else {
          bool z2zE984;
          {
            bool z2zE978;
            {
              bool z2zE976;
              {
                struct zAccount z2zE974;
                z2zE974 = zvalue.zcurr;
                z2zE976 = z2zE974.zpresent;
              }
              bool z2zE977;
              {
                struct zAccount z2zE975;
                z2zE975 = zvalue.zorig;
                z2zE977 = z2zE975.zpresent;
              }
              z2zE978 = eq_bool(z2zE976, z2zE977);
            }
            z2zE984 = not(z2zE978);
          }
          bool z3zE1122;
          if (z2zE984) {  z3zE1122 = true;  } else {
            bool z2zE983;
            {
              bool z2zE981;
              {
                struct zAccount z2zE979;
                z2zE979 = zvalue.zcurr;
                z2zE981 = z2zE979.zstorage_cleared;
              }
              bool z2zE982;
              {
                struct zAccount z2zE980;
                z2zE980 = zvalue.zorig;
                z2zE982 = z2zE980.zstorage_cleared;
              }
              z2zE983 = eq_bool(z2zE981, z2zE982);
            }
            z3zE1122 = not(z2zE983);
          }
          z3zE1123 = z3zE1122;
        }
        z3zE1124 = z3zE1123;
      }
      z3zE1125 = z3zE1124;
    }
    z3zE1126 = z3zE1125;
  }
  z8zE567 = z3zE1126;
end_function_1653: ;
  return z8zE567;
end_block_exception_1654: ;

  return false;
}

bool zstorage_value_changed(struct zStorageValue zvalue)
{
  bool z8zE568;
  bool z2zE945;
  {
    sail_u256 z2zE943;
    z2zE943 = zvalue.zcurr;
    sail_u256 z2zE944;
    z2zE944 = zvalue.zorig;
    z2zE945 = eq_u256(z2zE943, z2zE944);
  }
  z8zE568 = not(z2zE945);
end_function_1651: ;
  return z8zE568;
end_block_exception_1652: ;

  return false;
}

void znext_changed_storage_entry(struct zoptionzIRStorageTrieEntryzK *z8zE569, sail_fixed_bytes_20 zaddr)
{
  bool zsearching;
  zsearching = true;
  struct zoptionzIRStorageTrieEntryzK zresult;
  CREATE(zoptionzIRStorageTrieEntryzK)(&zresult);
  zNonezIRStorageTrieEntryzK(&zresult, UNIT);
  bool z3zE1119;
  unit z3zE1120;
while_1644: ;
  {
    z3zE1119 = zsearching;
    if (!(z3zE1119)) goto wend_1645;
    struct zoptionzIRStorageTrieEntryzK z2zE939;
    CREATE(zoptionzIRStorageTrieEntryzK)(&z2zE939);
    storage_block_iter_next(&z2zE939, zaddr);
    unit z3zE1115;
    {
      if (z2zE939.kind != Kind_zSomezIRStorageTrieEntryzK) goto case_1648;
      struct zStorageTrieEntry ztrie_entry;
      ztrie_entry = z2zE939.variants.zSomezIRStorageTrieEntryzK;
      bool z2zE942;
      {
        struct zStorageValue z2zE941;
        {
          struct zStorageEntry z2zE940;
          z2zE940 = ztrie_entry.zentry;
          z2zE941 = z2zE940.zvalue;
        }
        z2zE942 = zstorage_value_changed(z2zE941);
      }
      if (z2zE942) {
        zSomezIRStorageTrieEntryzK(&zresult, ztrie_entry);
        unit z3zE1118;
        z3zE1118 = UNIT;
        zsearching = false;
        z3zE1115 = UNIT;
      } else {  z3zE1115 = UNIT;  }
      goto finish_match_1646;
    }
  case_1648: ;
    {
      /* complete */
      zsearching = false;
      z3zE1115 = UNIT;
      goto finish_match_1646;
    }
  case_1647: ;
  finish_match_1646: ;
    z3zE1120 = z3zE1115;
    KILL(zoptionzIRStorageTrieEntryzK)(&z2zE939);
    goto while_1644;
  }
wend_1645: ;
  unit z3zE1121;
  z3zE1121 = UNIT;
  (*(z8zE569)) = zresult;
  KILL(zoptionzIRStorageTrieEntryzK)(&zresult);
end_function_1649: ;
  goto end_function_3616;
end_block_exception_1650: ;
  goto end_function_3616;
end_function_3616: ;
}

void zaccount_trie_update(struct ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9 *z8zE570, struct zAcctTrieEntry ztrie_entry)
{
  struct zAcctEntry zentry;
  zentry = ztrie_entry.zentry;
  sail_fixed_bytes_20 z2zE931;
  z2zE931 = zentry.zaddr;
  unit z3zE1109;
  z3zE1109 = storage_block_iter_begin(z2zE931);
  bool zstorage_changed;
  {
    struct zoptionzIRStorageTrieEntryzK z2zE938;
    CREATE(zoptionzIRStorageTrieEntryzK)(&z2zE938);
    {
      sail_fixed_bytes_20 z2zE937;
      z2zE937 = zentry.zaddr;
      znext_changed_storage_entry(&z2zE938, z2zE937);
    }
    bool z3zE1110;
    {
      if (z2zE938.kind != Kind_zSomezIRStorageTrieEntryzK) goto case_1641;
      struct zStorageTrieEntry z_storage_entry;
      z_storage_entry = z2zE938.variants.zSomezIRStorageTrieEntryzK;
      z3zE1110 = true;
      goto finish_match_1639;
    }
  case_1641: ;
    {
      /* complete */
      z3zE1110 = false;
      goto finish_match_1639;
    }
  case_1640: ;
  finish_match_1639: ;
    zstorage_changed = z3zE1110;
    KILL(zoptionzIRStorageTrieEntryzK)(&z2zE938);
  }
  sail_fixed_bytes_32 zstorage_root;
  {
    sail_fixed_bytes_20 z2zE936;
    z2zE936 = zentry.zaddr;
    zstorage_root = acct_post_storage_root_read(z2zE936);
  }
  struct zTrieUpdate z2zE934;
  CREATE(zTrieUpdate)(&z2zE934);
  zaccount_update(&z2zE934, ztrie_entry, zstorage_root);
  bool z2zE935;
  {
    bool z2zE933;
    {
      struct zAcctValue z2zE932;
      z2zE932 = zentry.zvalue;
      z2zE933 = zaccount_value_changed(z2zE932);
    }
    bool z3zE1113;
    if (z2zE933) {  z3zE1113 = true;  } else {  z3zE1113 = zstorage_changed;  }
    z2zE935 = z3zE1113;
  }
  struct ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9 z3zE1114;
  CREATE(ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9)(&z3zE1114);
  z3zE1114.ztup0 = z2zE934;
  z3zE1114.ztup1 = z2zE935;
  (*(z8zE570)) = z3zE1114;
  KILL(ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9)(&z3zE1114);
  KILL(zTrieUpdate)(&z2zE934);
end_function_1642: ;
  goto end_function_3615;
end_block_exception_1643: ;
  goto end_function_3615;
end_function_3615: ;
}

void znext_changed_account_trie_update(struct zoptionzIRTrieUpdatezK *z8zE571, unit z3zE1099)
{
  bool zsearching;
  zsearching = true;
  struct zoptionzIRTrieUpdatezK zresult;
  CREATE(zoptionzIRTrieUpdatezK)(&zresult);
  zNonezIRTrieUpdatezK(&zresult, UNIT);
  bool z3zE1106;
  unit z3zE1107;
while_1630: ;
  {
    z3zE1106 = zsearching;
    if (!(z3zE1106)) goto wend_1631;
    struct zoptionzIRAcctTrieEntryzK z2zE929;
    CREATE(zoptionzIRAcctTrieEntryzK)(&z2zE929);
    acct_block_iter_next(&z2zE929, UNIT);
    unit z3zE1100;
    {
      if (z2zE929.kind != Kind_zSomezIRAcctTrieEntryzK) goto case_1634;
      struct zAcctTrieEntry zentry;
      zentry = z2zE929.variants.zSomezIRAcctTrieEntryzK;
      struct ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9 z2zE930;
      CREATE(ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9)(&z2zE930);
      zaccount_trie_update(&z2zE930, zentry);
      unit z3zE1102;
      {
        struct zTrieUpdate zupdate;
        CREATE(zTrieUpdate)(&zupdate);
        zupdate = z2zE930.ztup0;
        bool zchanged;
        zchanged = z2zE930.ztup1;
        if (zchanged) {
          zSomezIRTrieUpdatezK(&zresult, zupdate);
          unit z3zE1104;
          z3zE1104 = UNIT;
          zsearching = false;
          z3zE1102 = UNIT;
        } else {  z3zE1102 = UNIT;  }
        KILL(zTrieUpdate)(&zupdate);
        goto finish_match_1635;
      }
    case_1636: ;
      sail_match_failure("next_changed_account_trie_update");
    finish_match_1635: ;
      z3zE1100 = z3zE1102;
      KILL(ztuple_z8z5structz0zzTrieUpdatezCz0z5boolz9)(&z2zE930);
      goto finish_match_1632;
    }
  case_1634: ;
    {
      /* complete */
      zsearching = false;
      z3zE1100 = UNIT;
      goto finish_match_1632;
    }
  case_1633: ;
  finish_match_1632: ;
    z3zE1107 = z3zE1100;
    KILL(zoptionzIRAcctTrieEntryzK)(&z2zE929);
    goto while_1630;
  }
wend_1631: ;
  unit z3zE1108;
  z3zE1108 = UNIT;
  (*(z8zE571)) = zresult;
  KILL(zoptionzIRTrieUpdatezK)(&zresult);
end_function_1637: ;
  goto end_function_3614;
end_block_exception_1638: ;
  goto end_function_3614;
end_function_3614: ;
}

struct zByteSliceFields zencode_storage_value(sail_u256 zvalue)
{
  struct zByteSliceFields z8zE572;
  uint64_t zencoded_len;
  zencoded_len = zrlp_uint_word_sizze(zvalue);
  uint64_t zstart;
  zstart = zscratch_begin(UNIT);
  unit z3zE1098;
  z3zE1098 = zrlp_write_uint_word(zvalue);
  z8zE572 = zrlp_finish(zstart);
end_function_1628: ;
  return z8zE572;
end_block_exception_1629: ;
  struct zByteSliceFields z8zE918 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE918;
}

void zstorage_update(struct zTrieUpdate *z8zE573, struct zStorageTrieEntry ztrie_entry)
{
  struct zStorageEntry zentry;
  zentry = ztrie_entry.zentry;
  struct zTriePath zkey;
  {
    sail_fixed_bytes_32 z2zE928;
    z2zE928 = ztrie_entry.zslot_hash;
    zkey = zpath_from_hash(z2zE928);
  }
  struct zTrieChange zchange;
  CREATE(zTrieChange)(&zchange);
  {
    bool z2zE924;
    {
      sail_u256 z2zE923;
      {
        struct zStorageValue z2zE922;
        z2zE922 = zentry.zvalue;
        z2zE923 = z2zE922.zcurr;
      }
      z2zE924 = zword_is_zzero(z2zE923);
    }
    if (z2zE924) {  zTrieDelete(&zchange, UNIT);  } else {
      struct zByteSliceFields z2zE927;
      {
        sail_u256 z2zE926;
        {
          struct zStorageValue z2zE925;
          z2zE925 = zentry.zvalue;
          z2zE926 = z2zE925.zcurr;
        }
        z2zE927 = zencode_storage_value(z2zE926);
      }
      zTriePut(&zchange, z2zE927);
    }
  }
  struct zTrieUpdate z3zE1097;
  CREATE(zTrieUpdate)(&z3zE1097);
  z3zE1097.zchange = zchange;
  z3zE1097.zkey = zkey;
  (*(z8zE573)) = z3zE1097;
  KILL(zTrieUpdate)(&z3zE1097);
  KILL(zTrieChange)(&zchange);
end_function_1626: ;
  goto end_function_3613;
end_block_exception_1627: ;
  goto end_function_3613;
end_function_3613: ;
}

void znext_storage_trie_update(struct zoptionzIRTrieUpdatezK *z8zE574, sail_fixed_bytes_20 zaddr)
{
  struct zoptionzIRStorageTrieEntryzK z2zE920;
  CREATE(zoptionzIRStorageTrieEntryzK)(&z2zE920);
  znext_changed_storage_entry(&z2zE920, zaddr);
  struct zoptionzIRTrieUpdatezK z3zE1094;
  CREATE(zoptionzIRTrieUpdatezK)(&z3zE1094);
  {
    if (z2zE920.kind != Kind_zSomezIRStorageTrieEntryzK) goto case_1623;
    struct zStorageTrieEntry zentry;
    zentry = z2zE920.variants.zSomezIRStorageTrieEntryzK;
    struct zTrieUpdate z2zE921;
    CREATE(zTrieUpdate)(&z2zE921);
    zstorage_update(&z2zE921, zentry);
    zSomezIRTrieUpdatezK(&z3zE1094, z2zE921);
    KILL(zTrieUpdate)(&z2zE921);
    goto finish_match_1621;
  }
case_1623: ;
  {
    /* complete */
    zNonezIRTrieUpdatezK(&z3zE1094, UNIT);
    goto finish_match_1621;
  }
case_1622: ;
finish_match_1621: ;
  (*(z8zE574)) = z3zE1094;
  KILL(zoptionzIRTrieUpdatezK)(&z3zE1094);
  KILL(zoptionzIRStorageTrieEntryzK)(&z2zE920);
end_function_1624: ;
  goto end_function_3612;
end_block_exception_1625: ;
  goto end_function_3612;
end_function_3612: ;
}

void ztrie_update_source_next(struct zoptionzIRTrieUpdatezK *z8zE575, struct zTrieUpdateSource zsource)
{
  struct zoptionzIRTrieUpdatezK z3zE1090;
  CREATE(zoptionzIRTrieUpdatezK)(&z3zE1090);
  {
    if (zsource.kind != Kind_zStorageTrieUpdates) goto case_1618;
    sail_fixed_bytes_20 zaddr;
    zaddr = zsource.variants.zStorageTrieUpdates;
    znext_storage_trie_update(&z3zE1090, zaddr);
    goto finish_match_1615;
  }
case_1618: ;
  {
    if (zsource.kind != Kind_zChangedAccountTrieUpdates) goto case_1617;
    znext_changed_account_trie_update(&z3zE1090, UNIT);
    goto finish_match_1615;
  }
case_1617: ;
  {
    /* complete */
    zcached_account_trie_update_next(&z3zE1090, UNIT);
    goto finish_match_1615;
  }
case_1616: ;
finish_match_1615: ;
  (*(z8zE575)) = z3zE1090;
  KILL(zoptionzIRTrieUpdatezK)(&z3zE1090);
end_function_1619: ;
  goto end_function_3611;
end_block_exception_1620: ;
  goto end_function_3611;
end_function_3611: ;
}

void ztrie_updates_begin(struct zTrieUpdateCursor *z8zE576, struct zTrieUpdateSource zsource)
{
  struct zoptionzIRTrieUpdatezK z2zE919;
  CREATE(zoptionzIRTrieUpdatezK)(&z2zE919);
  ztrie_update_source_next(&z2zE919, zsource);
  struct zTrieUpdateCursor z3zE1089;
  CREATE(zTrieUpdateCursor)(&z3zE1089);
  z3zE1089.zpending = z2zE919;
  z3zE1089.zsource = zsource;
  (*(z8zE576)) = z3zE1089;
  KILL(zTrieUpdateCursor)(&z3zE1089);
  KILL(zoptionzIRTrieUpdatezK)(&z2zE919);
end_function_1613: ;
  goto end_function_3610;
end_block_exception_1614: ;
  goto end_function_3610;
end_function_3610: ;
}

bool zupdates_empty(struct zTrieUpdateCursor zupdates)
{
  bool z8zE577;
  struct zoptionzIRTrieUpdatezK z2zE918;
  CREATE(zoptionzIRTrieUpdatezK)(&z2zE918);
  z2zE918 = zupdates.zpending;
  bool z3zE1086;
  {
    if (z2zE918.kind != Kind_zNonezIRTrieUpdatezK) goto case_1610;
    z3zE1086 = true;
    goto finish_match_1608;
  }
case_1610: ;
  {
    /* complete */
    z3zE1086 = false;
    goto finish_match_1608;
  }
case_1609: ;
finish_match_1608: ;
  z8zE577 = z3zE1086;
  KILL(zoptionzIRTrieUpdatezK)(&z2zE918);
end_function_1611: ;
  return z8zE577;
end_block_exception_1612: ;

  return false;
}

void ztrie_updates_advance(struct zTrieUpdateCursor *z8zE578, struct zTrieUpdateCursor zupdates)
{
  struct zTrieUpdateSource z2zE915;
  CREATE(zTrieUpdateSource)(&z2zE915);
  z2zE915 = zupdates.zsource;
  struct zoptionzIRTrieUpdatezK z2zE917;
  CREATE(zoptionzIRTrieUpdatezK)(&z2zE917);
  {
    struct zTrieUpdateSource z2zE916;
    CREATE(zTrieUpdateSource)(&z2zE916);
    z2zE916 = zupdates.zsource;
    ztrie_update_source_next(&z2zE917, z2zE916);
    KILL(zTrieUpdateSource)(&z2zE916);
  }
  struct zTrieUpdateCursor z3zE1085;
  CREATE(zTrieUpdateCursor)(&z3zE1085);
  z3zE1085.zpending = z2zE917;
  z3zE1085.zsource = z2zE915;
  (*(z8zE578)) = z3zE1085;
  KILL(zTrieUpdateCursor)(&z3zE1085);
  KILL(zoptionzIRTrieUpdatezK)(&z2zE917);
  KILL(zTrieUpdateSource)(&z2zE915);
end_function_1606: ;
  goto end_function_3609;
end_block_exception_1607: ;
  goto end_function_3609;
end_function_3609: ;
}

bool znext_update_under(struct zTrieUpdateCursor zupdates, struct zTriePath zprefix)
{
  bool z8zE579;
  struct zoptionzIRTrieUpdatezK z2zE913;
  CREATE(zoptionzIRTrieUpdatezK)(&z2zE913);
  z2zE913 = zupdates.zpending;
  bool z3zE1082;
  {
    if (z2zE913.kind != Kind_zSomezIRTrieUpdatezK) goto case_1603;
    struct zTrieUpdate zupdate;
    CREATE(zTrieUpdate)(&zupdate);
    zupdate = z2zE913.variants.zSomezIRTrieUpdatezK;
    struct zTriePath z2zE914;
    z2zE914 = zupdate.zkey;
    {
      z3zE1082 = zpath_prefix_of(zprefix, z2zE914);
      if (have_exception) {
        KILL(zoptionzIRTrieUpdatezK)(&z2zE913);
        KILL(zTrieUpdate)(&zupdate);
        goto end_block_exception_1605;
      }
    }
    KILL(zTrieUpdate)(&zupdate);
    goto finish_match_1601;
  }
case_1603: ;
  {
    /* complete */
    z3zE1082 = false;
    goto finish_match_1601;
  }
case_1602: ;
finish_match_1601: ;
  z8zE579 = z3zE1082;
  KILL(zoptionzIRTrieUpdatezK)(&z2zE913);
end_function_1604: ;
  return z8zE579;
end_block_exception_1605: ;

  return false;
}

void zitem_leaf(struct zTrieItem *z8zE580, struct zTriePath zpath, struct zByteSliceFields zvalue)
{
  struct zTrieItemValue z2zE912;
  CREATE(zTrieItemValue)(&z2zE912);
  zLeafItem(&z2zE912, zvalue);
  struct zTrieItem z3zE1081;
  CREATE(zTrieItem)(&z3zE1081);
  z3zE1081.zpath = zpath;
  z3zE1081.zvalue = z2zE912;
  (*(z8zE580)) = z3zE1081;
  KILL(zTrieItem)(&z3zE1081);
  KILL(zTrieItemValue)(&z2zE912);
end_function_1599: ;
  goto end_function_3608;
end_block_exception_1600: ;
  goto end_function_3608;
end_function_3608: ;
}

void zitem_branch(struct zTrieItem *z8zE581, struct zTriePath zpath, struct zNodeRef zchildref)
{
  struct zTrieItemValue z2zE911;
  CREATE(zTrieItemValue)(&z2zE911);
  zBranchItem(&z2zE911, zchildref);
  struct zTrieItem z3zE1080;
  CREATE(zTrieItem)(&z3zE1080);
  z3zE1080.zpath = zpath;
  z3zE1080.zvalue = z2zE911;
  (*(z8zE581)) = z3zE1080;
  KILL(zTrieItem)(&z3zE1080);
  KILL(zTrieItemValue)(&z2zE911);
end_function_1597: ;
  goto end_function_3607;
end_block_exception_1598: ;
  goto end_function_3607;
end_function_3607: ;
}

void zitem_subtree(struct zTrieItem *z8zE582, struct zTriePath zpath, struct zNodeRef zchildref)
{
  struct zTrieItemValue z2zE910;
  CREATE(zTrieItemValue)(&z2zE910);
  zSubtreeItem(&z2zE910, zchildref);
  struct zTrieItem z3zE1079;
  CREATE(zTrieItem)(&z3zE1079);
  z3zE1079.zpath = zpath;
  z3zE1079.zvalue = z2zE910;
  (*(z8zE582)) = z3zE1079;
  KILL(zTrieItem)(&z3zE1079);
  KILL(zTrieItemValue)(&z2zE910);
end_function_1595: ;
  goto end_function_3606;
end_block_exception_1596: ;
  goto end_function_3606;
end_function_3606: ;
}

void zitem_ref(struct zNodeRef *z8zE583, struct zTrieItem zit, uint64_t zdepth)
{
  struct zTriePath zsuffix;
  {
    struct zTriePath z2zE909;
    z2zE909 = zit.zpath;
    {
      zsuffix = zpath_drop(z2zE909, zdepth);
      if (have_exception) {  goto end_block_exception_1594;  }
    }
  }
  struct zTrieItemValue z2zE901;
  CREATE(zTrieItemValue)(&z2zE901);
  z2zE901 = zit.zvalue;
  struct zNodeRef z3zE1071;
  CREATE(zNodeRef)(&z3zE1071);
  {
    if (z2zE901.kind != Kind_zLeafItem) goto case_1592;
    struct zByteSliceFields zvalue;
    zvalue = z2zE901.variants.zLeafItem;
    {
      zleaf_child_ref(&z3zE1071, zsuffix, zvalue);
      if (have_exception) {
        KILL(zTrieItemValue)(&z2zE901);
        KILL(zNodeRef)(&z3zE1071);
        goto end_block_exception_1594;
      }
    }
    goto finish_match_1586;
  }
case_1592: ;
  {
    if (z2zE901.kind != Kind_zBranchItem) goto case_1591;
    struct zNodeRef zsubref;
    CREATE(zNodeRef)(&zsubref);
    zsubref = z2zE901.variants.zBranchItem;
    bool z2zE903;
    {
      uint64_t z2zE902;
      z2zE902 = zpath_len(zsuffix);
      z2zE903 = (z2zE902 == UINT64_C(0));
    }
    if (z2zE903) {  z3zE1071 = zsubref;  } else {
      {
        zmerge_ext_ref(&z3zE1071, zsuffix, zsubref);
        if (have_exception) {
          KILL(zTrieItemValue)(&z2zE901);
          KILL(zNodeRef)(&z3zE1071);
          KILL(zNodeRef)(&zsubref);
          goto end_block_exception_1594;
        }
      }
    }
    KILL(zNodeRef)(&zsubref);
    goto finish_match_1586;
  }
case_1591: ;
  {
    /* complete */
    struct zNodeRef z3zE1078;
    CREATE(zNodeRef)(&z3zE1078);
    z3zE1078 = z2zE901.variants.zSubtreeItem;
    bool z2zE905;
    {
      uint64_t z2zE904;
      z2zE904 = zpath_len(zsuffix);
      z2zE905 = (z2zE904 == UINT64_C(0));
    }
    if (z2zE905) {  z3zE1071 = z3zE1078;  } else {
      struct zNodeRef z3zE1073;
      CREATE(zNodeRef)(&z3zE1073);
      {
        if (z3zE1078.kind != Kind_zHashRef) goto case_1590;
        sail_fixed_bytes_32 zh;
        zh = z3zE1078.variants.zHashRef;
        struct zByteSliceFields znode;
        znode = znode_db_lookup(zh);
        bool z2zE907;
        {
          uint64_t z2zE906;
          z2zE906 = znode.zlen;
          z2zE907 = (z2zE906 == UINT64_C(0));
        }
        if (z2zE907) {
          struct zexception z2zE908;
          CREATE(zexception)(&z2zE908);
          zInvalidBlock(&z2zE908, zWitnessDeficient);
          COPY(zexception)(current_exception, z2zE908);
          have_exception = true;
          COPY(sail_string)(throw_location, "sail/lib/mpt/updates.sail:117.24-117.60");
          KILL(zTrieItemValue)(&z2zE901);
          KILL(zNodeRef)(&z3zE1071);
          KILL(zNodeRef)(&z3zE1078);
          KILL(zNodeRef)(&z3zE1073);
          KILL(zexception)(&z2zE908);
          goto end_block_exception_1594;
          /* unreachable after throw */
          KILL(zexception)(&z2zE908);
        } else {
          {
            zmerge_ext_node(&z3zE1073, zsuffix, znode);
            if (have_exception) {
              KILL(zTrieItemValue)(&z2zE901);
              KILL(zNodeRef)(&z3zE1071);
              KILL(zNodeRef)(&z3zE1078);
              KILL(zNodeRef)(&z3zE1073);
              goto end_block_exception_1594;
            }
          }
        }
        goto finish_match_1588;
      }
    case_1590: ;
      {
        {
          zmerge_ext_ref(&z3zE1073, zsuffix, z3zE1078);
          if (have_exception) {
            KILL(zTrieItemValue)(&z2zE901);
            KILL(zNodeRef)(&z3zE1071);
            KILL(zNodeRef)(&z3zE1078);
            KILL(zNodeRef)(&z3zE1073);
            goto end_block_exception_1594;
          }
        }
        goto finish_match_1588;
      }
    case_1589: ;
    finish_match_1588: ;
      z3zE1071 = z3zE1073;
      KILL(zNodeRef)(&z3zE1073);
    }
    KILL(zNodeRef)(&z3zE1078);
    goto finish_match_1586;
  }
case_1587: ;
finish_match_1586: ;
  (*(z8zE583)) = z3zE1071;
  KILL(zNodeRef)(&z3zE1071);
  KILL(zTrieItemValue)(&z2zE901);
end_function_1593: ;
  goto end_function_3605;
end_block_exception_1594: ;
  goto end_function_3605;
end_function_3605: ;
}

void zempty_trie_branch_frame(struct zTrieBranchFrame *z8zE584, uint64_t zdepth)
{
  zz5vecz8z5unionz0zzNodeRefz9 z2zE900;
  CREATE(zz5vecz8z5unionz0zzNodeRefz9)(&z2zE900);
  {
    struct zNodeRef z2zE899;
    CREATE(zNodeRef)(&z2zE899);
    zEmptyRef(&z2zE899, UNIT);
    fast_unsigned_vector_init_zz5vecz8z5unionz0zzNodeRefz9(&z2zE900, UINT64_C(16), z2zE899);
    KILL(zNodeRef)(&z2zE899);
  }
  struct zTrieBranchFrame z3zE1070;
  CREATE(zTrieBranchFrame)(&z3zE1070);
  COPY(zz5vecz8z5unionz0zzNodeRefz9)(&((&z3zE1070)->zchildren), z2zE900);
  z3zE1070.zdepth = zdepth;
  z3zE1070.zmask = UINT64_C(0x0000);
  COPY(zTrieBranchFrame)((*(&z8zE584)), z3zE1070);
  KILL(zTrieBranchFrame)(&z3zE1070);
  KILL(zz5vecz8z5unionz0zzNodeRefz9)(&z2zE900);
end_function_1584: ;
  goto end_function_3604;
end_block_exception_1585: ;
  goto end_function_3604;
end_function_3604: ;
}

void ztrie_builder_empty(struct zTrieBuilder *z8zE585, unit z3zE1067)
{
  struct zNodeRef z2zE898;
  CREATE(zNodeRef)(&z2zE898);
  zEmptyRef(&z2zE898, UNIT);
  struct zTrieBuilder z3zE1068;
  CREATE(zTrieBuilder)(&z3zE1068);
  z3zE1068.zcomplete = false;
  zz5listz8z5structz0zzTrieBranchFramez9 z3zE1069;
  CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z3zE1069);
  COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&((&z3zE1068)->zframes), z3zE1069);
  KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z3zE1069);
  z3zE1068.zroot = z2zE898;
  COPY(zTrieBuilder)((*(&z8zE585)), z3zE1068);
  KILL(zTrieBuilder)(&z3zE1068);
  KILL(zNodeRef)(&z2zE898);
end_function_1582: ;
  goto end_function_3603;
end_block_exception_1583: ;
  goto end_function_3603;
end_function_3603: ;
}

void ztrie_builder_push(struct zTrieBuilder *z8zE586, struct zTrieBuilder zbuilder, uint64_t zdepth)
{
  zz5listz8z5structz0zzTrieBranchFramez9 z2zE895;
  CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE895);
  {
    struct zTrieBranchFrame z2zE893;
    CREATE(zTrieBranchFrame)(&z2zE893);
    zempty_trie_branch_frame(&z2zE893, zdepth);
    zz5listz8z5structz0zzTrieBranchFramez9 z2zE894;
    CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE894);
    COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE894, zbuilder.zframes);
    zconsz3z5structz0zzTrieBranchFrame(&z2zE895, z2zE893, z2zE894);
    KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE894);
    KILL(zTrieBranchFrame)(&z2zE893);
  }
  struct zNodeRef z2zE896;
  CREATE(zNodeRef)(&z2zE896);
  z2zE896 = zbuilder.zroot;
  bool z2zE897;
  z2zE897 = zbuilder.zcomplete;
  struct zTrieBuilder z3zE1066;
  CREATE(zTrieBuilder)(&z3zE1066);
  z3zE1066.zcomplete = z2zE897;
  COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&((&z3zE1066)->zframes), z2zE895);
  z3zE1066.zroot = z2zE896;
  COPY(zTrieBuilder)((*(&z8zE586)), z3zE1066);
  KILL(zTrieBuilder)(&z3zE1066);
  KILL(zNodeRef)(&z2zE896);
  KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE895);
end_function_1580: ;
  goto end_function_3602;
end_block_exception_1581: ;
  goto end_function_3602;
end_function_3602: ;
}

void ztrie_builder_attach(struct zTrieBuilder *z8zE587, struct zTrieBuilder zbuilder, struct zTriePath zpath, struct zNodeRef zchild)
{
  zz5listz8z5structz0zzTrieBranchFramez9 z2zE878;
  CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE878);
  COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE878, zbuilder.zframes);
  struct zTrieBuilder z3zE1057;
  CREATE(zTrieBuilder)(&z3zE1057);
  {
    if (!((z2zE878 == NULL))) goto case_1577;
    struct zexception z2zE879;
    CREATE(zexception)(&z2zE879);
    zInvalidBlock(&z2zE879, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE879);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/updates.sail:181.16-181.52");
    KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE878);
    KILL(zTrieBuilder)(&z3zE1057);
    KILL(zexception)(&z2zE879);
    goto end_block_exception_1579;
    /* unreachable after throw */
    KILL(zexception)(&z2zE879);
    goto finish_match_1575;
  }
case_1577: ;
  {
    /* complete */
    struct zTrieBranchFrame zcurrent;
    CREATE(zTrieBranchFrame)(&zcurrent);
    COPY(zTrieBranchFrame)(&zcurrent, (*z2zE878).hd);
    zz5listz8z5structz0zzTrieBranchFramez9 zrest;
    CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&zrest);
    COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&zrest, (*z2zE878).tl);
    struct zTrieBranchFrame zframe;
    CREATE(zTrieBranchFrame)(&zframe);
    COPY(zTrieBranchFrame)(&zframe, zcurrent);
    uint64_t zdepth;
    zdepth = zframe.zdepth;
    bool z2zE881;
    {
      uint64_t z2zE880;
      z2zE880 = zpath_len(zpath);
      z2zE881 = (!(zdepth < z2zE880));
    }
    unit z3zE1058;
    if (z2zE881) {
      struct zexception z2zE882;
      CREATE(zexception)(&z2zE882);
      zInvalidBlock(&z2zE882, zWitnessDeficient);
      COPY(zexception)(current_exception, z2zE882);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/lib/mpt/updates.sail:186.16-186.52");
      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE878);
      KILL(zTrieBuilder)(&z3zE1057);
      KILL(zTrieBranchFrame)(&zcurrent);
      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&zrest);
      KILL(zTrieBranchFrame)(&zframe);
      KILL(zexception)(&z2zE882);
      goto end_block_exception_1579;
      /* unreachable after throw */
      KILL(zexception)(&z2zE882);
    } else {  z3zE1058 = UNIT;  }
    uint64_t zchild_index;
    zchild_index = zpath_nibble(zpath, zdepth);
    bool z2zE884;
    {
      uint64_t z2zE883;
      z2zE883 = zframe.zmask;
      z2zE884 = zbranch_mask_has(z2zE883, zchild_index);
    }
    unit z3zE1062;
    if (z2zE884) {
      struct zexception z2zE885;
      CREATE(zexception)(&z2zE885);
      zInvalidBlock(&z2zE885, zWitnessDeficient);
      COPY(zexception)(current_exception, z2zE885);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/lib/mpt/updates.sail:190.16-190.52");
      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE878);
      KILL(zTrieBuilder)(&z3zE1057);
      KILL(zTrieBranchFrame)(&zcurrent);
      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&zrest);
      KILL(zTrieBranchFrame)(&zframe);
      KILL(zexception)(&z2zE885);
      goto end_block_exception_1579;
      /* unreachable after throw */
      KILL(zexception)(&z2zE885);
    } else {  z3zE1062 = UNIT;  }
    uint64_t z2zE886;
    z2zE886 = zframe.zmask;
    zframe.zmask = zbranch_mask_set(z2zE886, zchild_index);
    unit z3zE1061;
    z3zE1061 = UNIT;
    zz5vecz8z5unionz0zzNodeRefz9 z2zE889;
    CREATE(zz5vecz8z5unionz0zzNodeRefz9)(&z2zE889);
    {
      zz5vecz8z5unionz0zzNodeRefz9 z2zE887;
      CREATE(zz5vecz8z5unionz0zzNodeRefz9)(&z2zE887);
      COPY(zz5vecz8z5unionz0zzNodeRefz9)(&z2zE887, zframe.zchildren);
      uint64_t z2zE888;
      z2zE888 = ((uint64_t) zchild_index);
      fast_unsigned_vector_update_zz5vecz8z5unionz0zzNodeRefz9(&z2zE889, z2zE887, z2zE888, zchild);
      KILL(zz5vecz8z5unionz0zzNodeRefz9)(&z2zE887);
    }
    COPY(zz5vecz8z5unionz0zzNodeRefz9)(&((&zframe)->zchildren), z2zE889);
    unit z3zE1060;
    z3zE1060 = UNIT;
    zz5listz8z5structz0zzTrieBranchFramez9 z2zE890;
    CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE890);
    zconsz3z5structz0zzTrieBranchFrame(&z2zE890, zframe, zrest);
    struct zNodeRef z2zE891;
    CREATE(zNodeRef)(&z2zE891);
    z2zE891 = zbuilder.zroot;
    bool z2zE892;
    z2zE892 = zbuilder.zcomplete;
    z3zE1057.zcomplete = z2zE892;
    COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&((&z3zE1057)->zframes), z2zE890);
    z3zE1057.zroot = z2zE891;
    goto finish_match_1575;
  }
case_1576: ;
finish_match_1575: ;
  COPY(zTrieBuilder)((*(&z8zE587)), z3zE1057);
  KILL(zTrieBuilder)(&z3zE1057);
  KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE878);
end_function_1578: ;
  goto end_function_3601;
end_block_exception_1579: ;
  goto end_function_3601;
end_function_3601: ;
}

void ztrie_builder_pop(struct ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9 *z8zE588, struct zTrieBuilder zbuilder)
{
  zz5listz8z5structz0zzTrieBranchFramez9 z2zE873;
  CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE873);
  COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE873, zbuilder.zframes);
  struct ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9 z3zE1052;
  CREATE(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z3zE1052);
  {
    if (!((z2zE873 == NULL))) goto case_1572;
    struct zexception z2zE874;
    CREATE(zexception)(&z2zE874);
    zInvalidBlock(&z2zE874, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE874);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/updates.sail:201.16-201.52");
    KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE873);
    KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z3zE1052);
    KILL(zexception)(&z2zE874);
    goto end_block_exception_1574;
    /* unreachable after throw */
    KILL(zexception)(&z2zE874);
    goto finish_match_1570;
  }
case_1572: ;
  {
    /* complete */
    struct zTrieBranchFrame zframe;
    CREATE(zTrieBranchFrame)(&zframe);
    COPY(zTrieBranchFrame)(&zframe, (*z2zE873).hd);
    zz5listz8z5structz0zzTrieBranchFramez9 zrest;
    CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&zrest);
    COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&zrest, (*z2zE873).tl);
    struct zTrieBuilder z2zE877;
    CREATE(zTrieBuilder)(&z2zE877);
    {
      struct zNodeRef z2zE875;
      CREATE(zNodeRef)(&z2zE875);
      z2zE875 = zbuilder.zroot;
      bool z2zE876;
      z2zE876 = zbuilder.zcomplete;
      z2zE877.zcomplete = z2zE876;
      COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&((&z2zE877)->zframes), zrest);
      z2zE877.zroot = z2zE875;
      KILL(zNodeRef)(&z2zE875);
    }
    COPY(zTrieBranchFrame)(&((&z3zE1052)->ztup0), zframe);
    COPY(zTrieBuilder)(&((&z3zE1052)->ztup1), z2zE877);
    KILL(zTrieBuilder)(&z2zE877);
    KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&zrest);
    KILL(zTrieBranchFrame)(&zframe);
    goto finish_match_1570;
  }
case_1571: ;
finish_match_1570: ;
  COPY(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)((*(&z8zE588)), z3zE1052);
  KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z3zE1052);
  KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE873);
end_function_1573: ;
  goto end_function_3600;
end_block_exception_1574: ;
  goto end_function_3600;
end_function_3600: ;
}

void ztrie_builder_wrap_branch(struct zNodeRef *z8zE589, struct zTriePath zanchor, uint64_t zparent_depth, uint64_t zchild_depth, struct zNodeRef zchild)
{
  uint64_t zchild_start;
  {    zchild_start = (zparent_depth + UINT64_C(1));
  }
  bool z2zE870;
  z2zE870 = (!(zchild_start < zchild_depth));
  if (z2zE870) {  (*(z8zE589)) = zchild;  } else {
    uint64_t zgap;
    {    zgap = (zchild_depth - zchild_start);
    }
    struct zTriePath z2zE872;
    {
      struct zTriePath z2zE871;
      {
        z2zE871 = zpath_drop(zanchor, zchild_start);
        if (have_exception) {  goto end_block_exception_1569;  }
      }
      {
        z2zE872 = zpath_take(z2zE871, zgap);
        if (have_exception) {  goto end_block_exception_1569;  }
      }
    }
    {
      zextension_child_ref((*(&z8zE589)), z2zE872, zchild);
      if (have_exception) {  goto end_block_exception_1569;  }
    }
  }
end_function_1568: ;
  goto end_function_3599;
end_block_exception_1569: ;
  goto end_function_3599;
end_function_3599: ;
}

void ztrie_builder_close(struct zTrieBuilder *z8zE590, struct zTrieBuilder zbuilder, struct zTriePath zanchor, struct zoptionzIU64zK znext_common, uint64_t zfuel)
{
  bool z2zE845;
  z2zE845 = (zfuel == UINT64_C(0));
  if (z2zE845) {
    struct zexception z2zE846;
    CREATE(zexception)(&z2zE846);
    zInvalidBlock(&z2zE846, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE846);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/updates.sail:234.8-234.44");
    KILL(zexception)(&z2zE846);
    goto end_block_exception_1567;
    /* unreachable after throw */
    KILL(zexception)(&z2zE846);
  } else {
    zz5listz8z5structz0zzTrieBranchFramez9 z2zE847;
    CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE847);
    COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE847, zbuilder.zframes);
    struct zTrieBuilder z3zE1031;
    CREATE(zTrieBuilder)(&z3zE1031);
    {
      if (!((z2zE847 == NULL))) goto case_1565;
      COPY(zTrieBuilder)(&z3zE1031, zbuilder);
      goto finish_match_1549;
    }
  case_1565: ;
    {
      /* complete */
      struct zTrieBranchFrame ztop;
      CREATE(zTrieBranchFrame)(&ztop);
      COPY(zTrieBranchFrame)(&ztop, (*z2zE847).hd);
      bool zshould_close;
      {
        bool z3zE1032;
        {
          if (znext_common.kind != Kind_zNonezIU64zK) goto case_1553;
          z3zE1032 = true;
          goto finish_match_1551;
        }
      case_1553: ;
        {
          /* complete */
          uint64_t zdepth;
          zdepth = znext_common.variants.zSomezIU64zK;
          uint64_t z2zE869;
          z2zE869 = ztop.zdepth;
          z3zE1032 = (zdepth < z2zE869);
          goto finish_match_1551;
        }
      case_1552: ;
      finish_match_1551: ;
        zshould_close = z3zE1032;
      }
      bool z2zE848;
      z2zE848 = not(zshould_close);
      if (z2zE848) {  COPY(zTrieBuilder)(&z3zE1031, zbuilder);  } else {
        struct ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9 z2zE849;
        CREATE(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE849);
        {
          ztrie_builder_pop(&z2zE849, zbuilder);
          if (have_exception) {
            KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE847);
            KILL(zTrieBuilder)(&z3zE1031);
            KILL(zTrieBranchFrame)(&ztop);
            KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE849);
            goto end_block_exception_1567;
          }
        }
        struct zTrieBuilder z3zE1036;
        CREATE(zTrieBuilder)(&z3zE1036);
        {
          struct zTrieBranchFrame zframe;
          CREATE(zTrieBranchFrame)(&zframe);
          COPY(zTrieBranchFrame)(&zframe, z2zE849.ztup0);
          struct zTrieBuilder zpopped;
          CREATE(zTrieBuilder)(&zpopped);
          COPY(zTrieBuilder)(&zpopped, z2zE849.ztup1);
          struct zNodeRef zchild;
          CREATE(zNodeRef)(&zchild);
          {
            uint64_t z2zE867;
            z2zE867 = zframe.zmask;
            zz5vecz8z5unionz0zzNodeRefz9 z2zE868;
            CREATE(zz5vecz8z5unionz0zzNodeRefz9)(&z2zE868);
            COPY(zz5vecz8z5unionz0zzNodeRefz9)(&z2zE868, zframe.zchildren);
            {
              zbranch_child_ref(&zchild, z2zE867, z2zE868);
              if (have_exception) {
                KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE847);
                KILL(zTrieBuilder)(&z3zE1031);
                KILL(zTrieBranchFrame)(&ztop);
                KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE849);
                KILL(zTrieBuilder)(&z3zE1036);
                KILL(zTrieBranchFrame)(&zframe);
                KILL(zTrieBuilder)(&zpopped);
                KILL(zNodeRef)(&zchild);
                KILL(zz5vecz8z5unionz0zzNodeRefz9)(&z2zE868);
                goto end_block_exception_1567;
              }
            }
            KILL(zz5vecz8z5unionz0zzNodeRefz9)(&z2zE868);
          }
          struct zTrieBuilder zwith_parent;
          CREATE(zTrieBuilder)(&zwith_parent);
          {
            zz5listz8z5structz0zzTrieBranchFramez9 z2zE851;
            CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE851);
            COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE851, zpopped.zframes);
            struct zTrieBuilder z3zE1037;
            CREATE(zTrieBuilder)(&z3zE1037);
            {
              if ((z2zE851 == NULL)) goto case_1561;
              struct zTrieBranchFrame zparent;
              CREATE(zTrieBranchFrame)(&zparent);
              COPY(zTrieBranchFrame)(&zparent, (*z2zE851).hd);
              struct zTrieBuilder z3zE1043;
              CREATE(zTrieBuilder)(&z3zE1043);
              {
                if (znext_common.kind != Kind_zSomezIU64zK) goto case_1564;
                uint64_t zcommon;
                zcommon = znext_common.variants.zSomezIU64zK;
                bool z2zE853;
                {
                  uint64_t z2zE852;
                  z2zE852 = zparent.zdepth;
                  z2zE853 = (z2zE852 < zcommon);
                }
                if (z2zE853) {
                  struct zTrieBuilder zintermediate;
                  CREATE(zTrieBuilder)(&zintermediate);
                  ztrie_builder_push(&zintermediate, zpopped, zcommon);
                  struct zNodeRef z2zE855;
                  CREATE(zNodeRef)(&z2zE855);
                  {
                    uint64_t z2zE854;
                    z2zE854 = zframe.zdepth;
                    {
                      ztrie_builder_wrap_branch(&z2zE855, zanchor, zcommon, z2zE854, zchild);
                      if (have_exception) {
                        KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE847);
                        KILL(zTrieBuilder)(&z3zE1031);
                        KILL(zTrieBranchFrame)(&ztop);
                        KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE849);
                        KILL(zTrieBuilder)(&z3zE1036);
                        KILL(zTrieBranchFrame)(&zframe);
                        KILL(zTrieBuilder)(&zpopped);
                        KILL(zNodeRef)(&zchild);
                        KILL(zTrieBuilder)(&zwith_parent);
                        KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE851);
                        KILL(zTrieBuilder)(&z3zE1037);
                        KILL(zTrieBranchFrame)(&zparent);
                        KILL(zTrieBuilder)(&z3zE1043);
                        KILL(zTrieBuilder)(&zintermediate);
                        KILL(zNodeRef)(&z2zE855);
                        goto end_block_exception_1567;
                      }
                    }
                  }
                  {
                    ztrie_builder_attach(&z3zE1043, zintermediate, zanchor, z2zE855);
                    if (have_exception) {
                      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE847);
                      KILL(zTrieBuilder)(&z3zE1031);
                      KILL(zTrieBranchFrame)(&ztop);
                      KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE849);
                      KILL(zTrieBuilder)(&z3zE1036);
                      KILL(zTrieBranchFrame)(&zframe);
                      KILL(zTrieBuilder)(&zpopped);
                      KILL(zNodeRef)(&zchild);
                      KILL(zTrieBuilder)(&zwith_parent);
                      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE851);
                      KILL(zTrieBuilder)(&z3zE1037);
                      KILL(zTrieBranchFrame)(&zparent);
                      KILL(zTrieBuilder)(&z3zE1043);
                      KILL(zTrieBuilder)(&zintermediate);
                      KILL(zNodeRef)(&z2zE855);
                      goto end_block_exception_1567;
                    }
                  }
                  KILL(zNodeRef)(&z2zE855);
                  KILL(zTrieBuilder)(&zintermediate);
                } else {
                  struct zNodeRef z2zE858;
                  CREATE(zNodeRef)(&z2zE858);
                  {
                    uint64_t z2zE856;
                    z2zE856 = zparent.zdepth;
                    uint64_t z2zE857;
                    z2zE857 = zframe.zdepth;
                    {
                      ztrie_builder_wrap_branch(&z2zE858, zanchor, z2zE856, z2zE857, zchild);
                      if (have_exception) {
                        KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE847);
                        KILL(zTrieBuilder)(&z3zE1031);
                        KILL(zTrieBranchFrame)(&ztop);
                        KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE849);
                        KILL(zTrieBuilder)(&z3zE1036);
                        KILL(zTrieBranchFrame)(&zframe);
                        KILL(zTrieBuilder)(&zpopped);
                        KILL(zNodeRef)(&zchild);
                        KILL(zTrieBuilder)(&zwith_parent);
                        KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE851);
                        KILL(zTrieBuilder)(&z3zE1037);
                        KILL(zTrieBranchFrame)(&zparent);
                        KILL(zTrieBuilder)(&z3zE1043);
                        KILL(zNodeRef)(&z2zE858);
                        goto end_block_exception_1567;
                      }
                    }
                  }
                  {
                    ztrie_builder_attach(&z3zE1043, zpopped, zanchor, z2zE858);
                    if (have_exception) {
                      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE847);
                      KILL(zTrieBuilder)(&z3zE1031);
                      KILL(zTrieBranchFrame)(&ztop);
                      KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE849);
                      KILL(zTrieBuilder)(&z3zE1036);
                      KILL(zTrieBranchFrame)(&zframe);
                      KILL(zTrieBuilder)(&zpopped);
                      KILL(zNodeRef)(&zchild);
                      KILL(zTrieBuilder)(&zwith_parent);
                      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE851);
                      KILL(zTrieBuilder)(&z3zE1037);
                      KILL(zTrieBranchFrame)(&zparent);
                      KILL(zTrieBuilder)(&z3zE1043);
                      KILL(zNodeRef)(&z2zE858);
                      goto end_block_exception_1567;
                    }
                  }
                  KILL(zNodeRef)(&z2zE858);
                }
                goto finish_match_1562;
              }
            case_1564: ;
              {
                /* complete */
                struct zNodeRef z2zE861;
                CREATE(zNodeRef)(&z2zE861);
                {
                  uint64_t z2zE859;
                  z2zE859 = zparent.zdepth;
                  uint64_t z2zE860;
                  z2zE860 = zframe.zdepth;
                  {
                    ztrie_builder_wrap_branch(&z2zE861, zanchor, z2zE859, z2zE860, zchild);
                    if (have_exception) {
                      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE847);
                      KILL(zTrieBuilder)(&z3zE1031);
                      KILL(zTrieBranchFrame)(&ztop);
                      KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE849);
                      KILL(zTrieBuilder)(&z3zE1036);
                      KILL(zTrieBranchFrame)(&zframe);
                      KILL(zTrieBuilder)(&zpopped);
                      KILL(zNodeRef)(&zchild);
                      KILL(zTrieBuilder)(&zwith_parent);
                      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE851);
                      KILL(zTrieBuilder)(&z3zE1037);
                      KILL(zTrieBranchFrame)(&zparent);
                      KILL(zTrieBuilder)(&z3zE1043);
                      KILL(zNodeRef)(&z2zE861);
                      goto end_block_exception_1567;
                    }
                  }
                }
                {
                  ztrie_builder_attach(&z3zE1043, zpopped, zanchor, z2zE861);
                  if (have_exception) {
                    KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE847);
                    KILL(zTrieBuilder)(&z3zE1031);
                    KILL(zTrieBranchFrame)(&ztop);
                    KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE849);
                    KILL(zTrieBuilder)(&z3zE1036);
                    KILL(zTrieBranchFrame)(&zframe);
                    KILL(zTrieBuilder)(&zpopped);
                    KILL(zNodeRef)(&zchild);
                    KILL(zTrieBuilder)(&zwith_parent);
                    KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE851);
                    KILL(zTrieBuilder)(&z3zE1037);
                    KILL(zTrieBranchFrame)(&zparent);
                    KILL(zTrieBuilder)(&z3zE1043);
                    KILL(zNodeRef)(&z2zE861);
                    goto end_block_exception_1567;
                  }
                }
                KILL(zNodeRef)(&z2zE861);
                goto finish_match_1562;
              }
            case_1563: ;
            finish_match_1562: ;
              COPY(zTrieBuilder)(&z3zE1037, z3zE1043);
              KILL(zTrieBuilder)(&z3zE1043);
              KILL(zTrieBranchFrame)(&zparent);
              goto finish_match_1556;
            }
          case_1561: ;
            {
              /* complete */
              struct zTrieBuilder z3zE1038;
              CREATE(zTrieBuilder)(&z3zE1038);
              {
                if (znext_common.kind != Kind_zSomezIU64zK) goto case_1560;
                uint64_t z3zE1049;
                z3zE1049 = znext_common.variants.zSomezIU64zK;
                struct zTrieBuilder z3zE1050;
                CREATE(zTrieBuilder)(&z3zE1050);
                ztrie_builder_push(&z3zE1050, zpopped, z3zE1049);
                struct zNodeRef z2zE863;
                CREATE(zNodeRef)(&z2zE863);
                {
                  uint64_t z2zE862;
                  z2zE862 = zframe.zdepth;
                  {
                    ztrie_builder_wrap_branch(&z2zE863, zanchor, z3zE1049, z2zE862, zchild);
                    if (have_exception) {
                      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE847);
                      KILL(zTrieBuilder)(&z3zE1031);
                      KILL(zTrieBranchFrame)(&ztop);
                      KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE849);
                      KILL(zTrieBuilder)(&z3zE1036);
                      KILL(zTrieBranchFrame)(&zframe);
                      KILL(zTrieBuilder)(&zpopped);
                      KILL(zNodeRef)(&zchild);
                      KILL(zTrieBuilder)(&zwith_parent);
                      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE851);
                      KILL(zTrieBuilder)(&z3zE1037);
                      KILL(zTrieBuilder)(&z3zE1038);
                      KILL(zTrieBuilder)(&z3zE1050);
                      KILL(zNodeRef)(&z2zE863);
                      goto end_block_exception_1567;
                    }
                  }
                }
                {
                  ztrie_builder_attach(&z3zE1038, z3zE1050, zanchor, z2zE863);
                  if (have_exception) {
                    KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE847);
                    KILL(zTrieBuilder)(&z3zE1031);
                    KILL(zTrieBranchFrame)(&ztop);
                    KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE849);
                    KILL(zTrieBuilder)(&z3zE1036);
                    KILL(zTrieBranchFrame)(&zframe);
                    KILL(zTrieBuilder)(&zpopped);
                    KILL(zNodeRef)(&zchild);
                    KILL(zTrieBuilder)(&zwith_parent);
                    KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE851);
                    KILL(zTrieBuilder)(&z3zE1037);
                    KILL(zTrieBuilder)(&z3zE1038);
                    KILL(zTrieBuilder)(&z3zE1050);
                    KILL(zNodeRef)(&z2zE863);
                    goto end_block_exception_1567;
                  }
                }
                KILL(zNodeRef)(&z2zE863);
                KILL(zTrieBuilder)(&z3zE1050);
                goto finish_match_1558;
              }
            case_1560: ;
              {
                /* complete */
                uint64_t z3zE1051;
                z3zE1051 = zframe.zdepth;
                struct zNodeRef zroot;
                CREATE(zNodeRef)(&zroot);
                {
                  bool z2zE865;
                  z2zE865 = (z3zE1051 == UINT64_C(0));
                  if (z2zE865) {  zroot = zchild;  } else {
                    struct zTriePath z2zE866;
                    {
                      z2zE866 = zpath_take(zanchor, z3zE1051);
                      if (have_exception) {
                        KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE847);
                        KILL(zTrieBuilder)(&z3zE1031);
                        KILL(zTrieBranchFrame)(&ztop);
                        KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE849);
                        KILL(zTrieBuilder)(&z3zE1036);
                        KILL(zTrieBranchFrame)(&zframe);
                        KILL(zTrieBuilder)(&zpopped);
                        KILL(zNodeRef)(&zchild);
                        KILL(zTrieBuilder)(&zwith_parent);
                        KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE851);
                        KILL(zTrieBuilder)(&z3zE1037);
                        KILL(zTrieBuilder)(&z3zE1038);
                        KILL(zNodeRef)(&zroot);
                        goto end_block_exception_1567;
                      }
                    }
                    {
                      zextension_child_ref(&zroot, z2zE866, zchild);
                      if (have_exception) {
                        KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE847);
                        KILL(zTrieBuilder)(&z3zE1031);
                        KILL(zTrieBranchFrame)(&ztop);
                        KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE849);
                        KILL(zTrieBuilder)(&z3zE1036);
                        KILL(zTrieBranchFrame)(&zframe);
                        KILL(zTrieBuilder)(&zpopped);
                        KILL(zNodeRef)(&zchild);
                        KILL(zTrieBuilder)(&zwith_parent);
                        KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE851);
                        KILL(zTrieBuilder)(&z3zE1037);
                        KILL(zTrieBuilder)(&z3zE1038);
                        KILL(zNodeRef)(&zroot);
                        goto end_block_exception_1567;
                      }
                    }
                  }
                }
                zz5listz8z5structz0zzTrieBranchFramez9 z2zE864;
                CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE864);
                COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE864, zpopped.zframes);
                z3zE1038.zcomplete = true;
                COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&((&z3zE1038)->zframes), z2zE864);
                z3zE1038.zroot = zroot;
                KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE864);
                KILL(zNodeRef)(&zroot);
                goto finish_match_1558;
              }
            case_1559: ;
            finish_match_1558: ;
              COPY(zTrieBuilder)(&z3zE1037, z3zE1038);
              KILL(zTrieBuilder)(&z3zE1038);
              goto finish_match_1556;
            }
          case_1557: ;
          finish_match_1556: ;
            COPY(zTrieBuilder)(&zwith_parent, z3zE1037);
            KILL(zTrieBuilder)(&z3zE1037);
            KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE851);
          }
          uint64_t z2zE850;
          {    z2zE850 = (zfuel - UINT64_C(1));
          }
          {
            ztrie_builder_close(&z3zE1036, zwith_parent, zanchor, znext_common, z2zE850);
            if (have_exception) {
              KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE847);
              KILL(zTrieBuilder)(&z3zE1031);
              KILL(zTrieBranchFrame)(&ztop);
              KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE849);
              KILL(zTrieBuilder)(&z3zE1036);
              KILL(zTrieBranchFrame)(&zframe);
              KILL(zTrieBuilder)(&zpopped);
              KILL(zNodeRef)(&zchild);
              KILL(zTrieBuilder)(&zwith_parent);
              goto end_block_exception_1567;
            }
          }
          KILL(zTrieBuilder)(&zwith_parent);
          KILL(zNodeRef)(&zchild);
          KILL(zTrieBuilder)(&zpopped);
          KILL(zTrieBranchFrame)(&zframe);
          goto finish_match_1554;
        }
      case_1555: ;
        sail_match_failure("trie_builder_close");
      finish_match_1554: ;
        COPY(zTrieBuilder)(&z3zE1031, z3zE1036);
        KILL(zTrieBuilder)(&z3zE1036);
        KILL(ztuple_z8z5structz0zzTrieBranchFramezCz0z5structz0zzTrieBuilderz9)(&z2zE849);
      }
      KILL(zTrieBranchFrame)(&ztop);
      goto finish_match_1549;
    }
  case_1550: ;
  finish_match_1549: ;
    COPY(zTrieBuilder)((*(&z8zE590)), z3zE1031);
    KILL(zTrieBuilder)(&z3zE1031);
    KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE847);
  }
end_function_1566: ;
  goto end_function_3598;
end_block_exception_1567: ;
  goto end_function_3598;
end_function_3598: ;
}

void ztrie_item_next_common(struct zoptionzIU64zK *z8zE591, struct zTrieItem zitem, struct zoptionzIRTriePathzK znext_key)
{
  struct zoptionzIU64zK z3zE1025;
  CREATE(zoptionzIU64zK)(&z3zE1025);
  {
    if (znext_key.kind != Kind_zNonezIRTriePathzK) goto case_1546;
    zNonezIU64zK(&z3zE1025, UNIT);
    goto finish_match_1544;
  }
case_1546: ;
  {
    /* complete */
    struct zTriePath znext;
    znext = znext_key.variants.zSomezIRTriePathzK;
    uint64_t zcommon;
    {
      struct zTriePath z2zE844;
      z2zE844 = zitem.zpath;
      {
        zcommon = zcommon_prefix_from(z2zE844, znext, UINT64_C(0));
        if (have_exception) {
          KILL(zoptionzIU64zK)(&z3zE1025);
          goto end_block_exception_1548;
        }
      }
    }
    bool z2zE841;
    {
      bool z2zE840;
      {
        bool z2zE835;
        {
          struct zTriePath z2zE834;
          z2zE834 = zitem.zpath;
          z2zE835 = zpath_lt(z2zE834, znext);
        }
        z2zE840 = not(z2zE835);
      }
      bool z3zE1027;
      if (z2zE840) {  z3zE1027 = true;  } else {
        bool z2zE839;
        {
          uint64_t z2zE837;
          {
            struct zTriePath z2zE836;
            z2zE836 = zitem.zpath;
            z2zE837 = zpath_len(z2zE836);
          }
          z2zE839 = (!(zcommon < z2zE837));
        }
        bool z3zE1026;
        if (z2zE839) {  z3zE1026 = true;  } else {
          uint64_t z2zE838;
          z2zE838 = zpath_len(znext);
          z3zE1026 = (!(zcommon < z2zE838));
        }
        z3zE1027 = z3zE1026;
      }
      z2zE841 = z3zE1027;
    }
    unit z3zE1028;
    if (z2zE841) {
      struct zexception z2zE842;
      CREATE(zexception)(&z2zE842);
      zInvalidBlock(&z2zE842, zWitnessDeficient);
      COPY(zexception)(current_exception, z2zE842);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/lib/mpt/updates.sail:305.16-305.52");
      KILL(zoptionzIU64zK)(&z3zE1025);
      KILL(zexception)(&z2zE842);
      goto end_block_exception_1548;
      /* unreachable after throw */
      KILL(zexception)(&z2zE842);
    } else {  z3zE1028 = UNIT;  }
    uint64_t z2zE843;
    {
      z2zE843 = zto_trie_depth(zcommon);
      if (have_exception) {
        KILL(zoptionzIU64zK)(&z3zE1025);
        goto end_block_exception_1548;
      }
    }
    zSomezIU64zK(&z3zE1025, z2zE843);
    goto finish_match_1544;
  }
case_1545: ;
finish_match_1544: ;
  (*(z8zE591)) = z3zE1025;
  KILL(zoptionzIU64zK)(&z3zE1025);
end_function_1547: ;
  goto end_function_3597;
end_block_exception_1548: ;
  goto end_function_3597;
end_function_3597: ;
}

void ztrie_insert_item(struct zTrieBuilder *z8zE592, struct zTrieBuilder zbuilder, struct zTrieItem zitem, struct zoptionzIRTriePathzK znext_key)
{
  bool z2zE818;
  z2zE818 = zbuilder.zcomplete;
  unit z3zE1009;
  if (z2zE818) {
    struct zexception z2zE819;
    CREATE(zexception)(&z2zE819);
    zInvalidBlock(&z2zE819, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE819);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/updates.sail:315.8-315.44");
    KILL(zexception)(&z2zE819);
    goto end_block_exception_1543;
    /* unreachable after throw */
    KILL(zexception)(&z2zE819);
  } else {  z3zE1009 = UNIT;  }
  struct zoptionzIU64zK znext_common;
  CREATE(zoptionzIU64zK)(&znext_common);
  {
    ztrie_item_next_common(&znext_common, zitem, znext_key);
    if (have_exception) {
      KILL(zoptionzIU64zK)(&znext_common);
      goto end_block_exception_1543;
    }
  }
  bool zopen_child;
  {
    bool z3zE1010;
    {
      if (znext_common.kind != Kind_zNonezIU64zK) goto case_1535;
      z3zE1010 = false;
      goto finish_match_1530;
    }
  case_1535: ;
    {
      /* complete */
      uint64_t zcommon;
      zcommon = znext_common.variants.zSomezIU64zK;
      zz5listz8z5structz0zzTrieBranchFramez9 z2zE832;
      CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE832);
      COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE832, zbuilder.zframes);
      bool z3zE1011;
      {
        if (!((z2zE832 == NULL))) goto case_1534;
        z3zE1011 = true;
        goto finish_match_1532;
      }
    case_1534: ;
      {
        /* complete */
        struct zTrieBranchFrame ztop;
        CREATE(zTrieBranchFrame)(&ztop);
        COPY(zTrieBranchFrame)(&ztop, (*z2zE832).hd);
        uint64_t z2zE833;
        z2zE833 = ztop.zdepth;
        z3zE1011 = (z2zE833 < zcommon);
        KILL(zTrieBranchFrame)(&ztop);
        goto finish_match_1532;
      }
    case_1533: ;
    finish_match_1532: ;
      z3zE1010 = z3zE1011;
      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE832);
      goto finish_match_1530;
    }
  case_1531: ;
  finish_match_1530: ;
    zopen_child = z3zE1010;
  }
  if (zopen_child) {
    struct zTrieBuilder z3zE1020;
    CREATE(zTrieBuilder)(&z3zE1020);
    {
      if (znext_common.kind != Kind_zSomezIU64zK) goto case_1541;
      uint64_t z3zE1023;
      z3zE1023 = znext_common.variants.zSomezIU64zK;
      struct zTrieBuilder zopened;
      CREATE(zTrieBuilder)(&zopened);
      ztrie_builder_push(&zopened, zbuilder, z3zE1023);
      struct zTriePath z2zE821;
      z2zE821 = zitem.zpath;
      struct zNodeRef z2zE822;
      CREATE(zNodeRef)(&z2zE822);
      {
        uint64_t z2zE820;
        {    z2zE820 = (z3zE1023 + UINT64_C(1));
        }
        {
          zitem_ref(&z2zE822, zitem, z2zE820);
          if (have_exception) {
            KILL(zoptionzIU64zK)(&znext_common);
            KILL(zTrieBuilder)(&z3zE1020);
            KILL(zTrieBuilder)(&zopened);
            KILL(zNodeRef)(&z2zE822);
            goto end_block_exception_1543;
          }
        }
      }
      {
        ztrie_builder_attach(&z3zE1020, zopened, z2zE821, z2zE822);
        if (have_exception) {
          KILL(zoptionzIU64zK)(&znext_common);
          KILL(zTrieBuilder)(&z3zE1020);
          KILL(zTrieBuilder)(&zopened);
          KILL(zNodeRef)(&z2zE822);
          goto end_block_exception_1543;
        }
      }
      KILL(zNodeRef)(&z2zE822);
      KILL(zTrieBuilder)(&zopened);
      goto finish_match_1539;
    }
  case_1541: ;
    {
      /* complete */
      struct zexception z2zE823;
      CREATE(zexception)(&z2zE823);
      zInvalidBlock(&z2zE823, zWitnessDeficient);
      COPY(zexception)(current_exception, z2zE823);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/lib/mpt/updates.sail:331.22-331.58");
      KILL(zoptionzIU64zK)(&znext_common);
      KILL(zTrieBuilder)(&z3zE1020);
      KILL(zexception)(&z2zE823);
      goto end_block_exception_1543;
      /* unreachable after throw */
      KILL(zexception)(&z2zE823);
      goto finish_match_1539;
    }
  case_1540: ;
  finish_match_1539: ;
    COPY(zTrieBuilder)((*(&z8zE592)), z3zE1020);
    KILL(zTrieBuilder)(&z3zE1020);
  } else {
    zz5listz8z5structz0zzTrieBranchFramez9 z2zE824;
    CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE824);
    COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE824, zbuilder.zframes);
    struct zTrieBuilder z3zE1016;
    CREATE(zTrieBuilder)(&z3zE1016);
    {
      if (!((z2zE824 == NULL))) goto case_1538;
      zz5listz8z5structz0zzTrieBranchFramez9 z2zE825;
      CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE825);
      COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE825, zbuilder.zframes);
      struct zNodeRef z2zE826;
      CREATE(zNodeRef)(&z2zE826);
      {
        zitem_ref(&z2zE826, zitem, UINT64_C(0));
        if (have_exception) {
          KILL(zoptionzIU64zK)(&znext_common);
          KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE824);
          KILL(zTrieBuilder)(&z3zE1016);
          KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE825);
          KILL(zNodeRef)(&z2zE826);
          goto end_block_exception_1543;
        }
      }
      z3zE1016.zcomplete = true;
      COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&((&z3zE1016)->zframes), z2zE825);
      z3zE1016.zroot = z2zE826;
      KILL(zNodeRef)(&z2zE826);
      KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE825);
      goto finish_match_1536;
    }
  case_1538: ;
    {
      /* complete */
      struct zTrieBranchFrame z3zE1024;
      CREATE(zTrieBranchFrame)(&z3zE1024);
      COPY(zTrieBranchFrame)(&z3zE1024, (*z2zE824).hd);
      struct zTrieBuilder zattached;
      CREATE(zTrieBuilder)(&zattached);
      {
        struct zTriePath z2zE830;
        z2zE830 = zitem.zpath;
        struct zNodeRef z2zE831;
        CREATE(zNodeRef)(&z2zE831);
        {
          uint64_t z2zE829;
          {
            uint64_t z2zE828;
            z2zE828 = z3zE1024.zdepth;
            {    z2zE829 = (z2zE828 + UINT64_C(1));
            }
          }
          {
            zitem_ref(&z2zE831, zitem, z2zE829);
            if (have_exception) {
              KILL(zoptionzIU64zK)(&znext_common);
              KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE824);
              KILL(zTrieBuilder)(&z3zE1016);
              KILL(zTrieBranchFrame)(&z3zE1024);
              KILL(zTrieBuilder)(&zattached);
              KILL(zNodeRef)(&z2zE831);
              goto end_block_exception_1543;
            }
          }
        }
        {
          ztrie_builder_attach(&zattached, zbuilder, z2zE830, z2zE831);
          if (have_exception) {
            KILL(zoptionzIU64zK)(&znext_common);
            KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE824);
            KILL(zTrieBuilder)(&z3zE1016);
            KILL(zTrieBranchFrame)(&z3zE1024);
            KILL(zTrieBuilder)(&zattached);
            KILL(zNodeRef)(&z2zE831);
            goto end_block_exception_1543;
          }
        }
        KILL(zNodeRef)(&z2zE831);
      }
      struct zTriePath z2zE827;
      z2zE827 = zitem.zpath;
      {
        ztrie_builder_close(&z3zE1016, zattached, z2zE827, znext_common, UINT64_C(64));
        if (have_exception) {
          KILL(zoptionzIU64zK)(&znext_common);
          KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE824);
          KILL(zTrieBuilder)(&z3zE1016);
          KILL(zTrieBranchFrame)(&z3zE1024);
          KILL(zTrieBuilder)(&zattached);
          goto end_block_exception_1543;
        }
      }
      KILL(zTrieBuilder)(&zattached);
      KILL(zTrieBranchFrame)(&z3zE1024);
      goto finish_match_1536;
    }
  case_1537: ;
  finish_match_1536: ;
    COPY(zTrieBuilder)((*(&z8zE592)), z3zE1016);
    KILL(zTrieBuilder)(&z3zE1016);
    KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE824);
  }
  KILL(zoptionzIU64zK)(&znext_common);
end_function_1542: ;
  goto end_function_3596;
end_block_exception_1543: ;
  goto end_function_3596;
end_function_3596: ;
}

void ztrie_sink_empty(struct zTrieItemSink *z8zE593, unit z3zE1007)
{
  struct zTrieBuilder z2zE816;
  CREATE(zTrieBuilder)(&z2zE816);
  ztrie_builder_empty(&z2zE816, UNIT);
  struct zoptionzIRTrieItemzK z2zE817;
  CREATE(zoptionzIRTrieItemzK)(&z2zE817);
  zNonezIRTrieItemzK(&z2zE817, UNIT);
  struct zTrieItemSink z3zE1008;
  CREATE(zTrieItemSink)(&z3zE1008);
  COPY(zTrieBuilder)(&((&z3zE1008)->zbuilder), z2zE816);
  z3zE1008.zpending = z2zE817;
  COPY(zTrieItemSink)((*(&z8zE593)), z3zE1008);
  KILL(zTrieItemSink)(&z3zE1008);
  KILL(zoptionzIRTrieItemzK)(&z2zE817);
  KILL(zTrieBuilder)(&z2zE816);
end_function_1528: ;
  goto end_function_3595;
end_block_exception_1529: ;
  goto end_function_3595;
end_function_3595: ;
}

void ztrie_sink_emit(struct zTrieItemSink *z8zE594, struct zTrieItemSink zsink, struct zTrieItem zitem)
{
  struct zoptionzIRTrieItemzK z2zE808;
  CREATE(zoptionzIRTrieItemzK)(&z2zE808);
  z2zE808 = zsink.zpending;
  struct zTrieItemSink z3zE1002;
  CREATE(zTrieItemSink)(&z3zE1002);
  {
    if (z2zE808.kind != Kind_zNonezIRTrieItemzK) goto case_1525;
    struct zTrieBuilder z2zE809;
    CREATE(zTrieBuilder)(&z2zE809);
    COPY(zTrieBuilder)(&z2zE809, zsink.zbuilder);
    struct zoptionzIRTrieItemzK z2zE810;
    CREATE(zoptionzIRTrieItemzK)(&z2zE810);
    zSomezIRTrieItemzK(&z2zE810, zitem);
    COPY(zTrieBuilder)(&((&z3zE1002)->zbuilder), z2zE809);
    z3zE1002.zpending = z2zE810;
    KILL(zoptionzIRTrieItemzK)(&z2zE810);
    KILL(zTrieBuilder)(&z2zE809);
    goto finish_match_1523;
  }
case_1525: ;
  {
    /* complete */
    struct zTrieItem zprevious;
    CREATE(zTrieItem)(&zprevious);
    zprevious = z2zE808.variants.zSomezIRTrieItemzK;
    struct zTrieBuilder z2zE814;
    CREATE(zTrieBuilder)(&z2zE814);
    {
      struct zTrieBuilder z2zE812;
      CREATE(zTrieBuilder)(&z2zE812);
      COPY(zTrieBuilder)(&z2zE812, zsink.zbuilder);
      struct zoptionzIRTriePathzK z2zE813;
      CREATE(zoptionzIRTriePathzK)(&z2zE813);
      {
        struct zTriePath z2zE811;
        z2zE811 = zitem.zpath;
        zSomezIRTriePathzK(&z2zE813, z2zE811);
      }
      {
        ztrie_insert_item(&z2zE814, z2zE812, zprevious, z2zE813);
        if (have_exception) {
          KILL(zoptionzIRTrieItemzK)(&z2zE808);
          KILL(zTrieItemSink)(&z3zE1002);
          KILL(zTrieItem)(&zprevious);
          KILL(zTrieBuilder)(&z2zE814);
          KILL(zTrieBuilder)(&z2zE812);
          KILL(zoptionzIRTriePathzK)(&z2zE813);
          goto end_block_exception_1527;
        }
      }
      KILL(zoptionzIRTriePathzK)(&z2zE813);
      KILL(zTrieBuilder)(&z2zE812);
    }
    struct zoptionzIRTrieItemzK z2zE815;
    CREATE(zoptionzIRTrieItemzK)(&z2zE815);
    zSomezIRTrieItemzK(&z2zE815, zitem);
    COPY(zTrieBuilder)(&((&z3zE1002)->zbuilder), z2zE814);
    z3zE1002.zpending = z2zE815;
    KILL(zoptionzIRTrieItemzK)(&z2zE815);
    KILL(zTrieBuilder)(&z2zE814);
    KILL(zTrieItem)(&zprevious);
    goto finish_match_1523;
  }
case_1524: ;
finish_match_1523: ;
  COPY(zTrieItemSink)((*(&z8zE594)), z3zE1002);
  KILL(zTrieItemSink)(&z3zE1002);
  KILL(zoptionzIRTrieItemzK)(&z2zE808);
end_function_1526: ;
  goto end_function_3594;
end_block_exception_1527: ;
  goto end_function_3594;
end_function_3594: ;
}

void ztrie_sink_finish(struct zTrieItemSink *z8zE595, struct zTrieItemSink zsink)
{
  struct zoptionzIRTrieItemzK z2zE803;
  CREATE(zoptionzIRTrieItemzK)(&z2zE803);
  z2zE803 = zsink.zpending;
  struct zTrieItemSink z3zE998;
  CREATE(zTrieItemSink)(&z3zE998);
  {
    if (z2zE803.kind != Kind_zNonezIRTrieItemzK) goto case_1520;
    COPY(zTrieItemSink)(&z3zE998, zsink);
    goto finish_match_1518;
  }
case_1520: ;
  {
    /* complete */
    struct zTrieItem zitem;
    CREATE(zTrieItem)(&zitem);
    zitem = z2zE803.variants.zSomezIRTrieItemzK;
    struct zTrieBuilder z2zE806;
    CREATE(zTrieBuilder)(&z2zE806);
    {
      struct zTrieBuilder z2zE804;
      CREATE(zTrieBuilder)(&z2zE804);
      COPY(zTrieBuilder)(&z2zE804, zsink.zbuilder);
      struct zoptionzIRTriePathzK z2zE805;
      CREATE(zoptionzIRTriePathzK)(&z2zE805);
      zNonezIRTriePathzK(&z2zE805, UNIT);
      {
        ztrie_insert_item(&z2zE806, z2zE804, zitem, z2zE805);
        if (have_exception) {
          KILL(zoptionzIRTrieItemzK)(&z2zE803);
          KILL(zTrieItemSink)(&z3zE998);
          KILL(zTrieItem)(&zitem);
          KILL(zTrieBuilder)(&z2zE806);
          KILL(zTrieBuilder)(&z2zE804);
          KILL(zoptionzIRTriePathzK)(&z2zE805);
          goto end_block_exception_1522;
        }
      }
      KILL(zoptionzIRTriePathzK)(&z2zE805);
      KILL(zTrieBuilder)(&z2zE804);
    }
    struct zoptionzIRTrieItemzK z2zE807;
    CREATE(zoptionzIRTrieItemzK)(&z2zE807);
    zNonezIRTrieItemzK(&z2zE807, UNIT);
    COPY(zTrieBuilder)(&((&z3zE998)->zbuilder), z2zE806);
    z3zE998.zpending = z2zE807;
    KILL(zoptionzIRTrieItemzK)(&z2zE807);
    KILL(zTrieBuilder)(&z2zE806);
    KILL(zTrieItem)(&zitem);
    goto finish_match_1518;
  }
case_1519: ;
finish_match_1518: ;
  COPY(zTrieItemSink)((*(&z8zE595)), z3zE998);
  KILL(zTrieItemSink)(&z3zE998);
  KILL(zoptionzIRTrieItemzK)(&z2zE803);
end_function_1521: ;
  goto end_function_3593;
end_block_exception_1522: ;
  goto end_function_3593;
end_function_3593: ;
}

sail_fixed_bytes_32 ztrie_builder_root(struct zTrieBuilder zbuilder)
{
  sail_fixed_bytes_32 z8zE596;
  zz5listz8z5structz0zzTrieBranchFramez9 z2zE799;
  CREATE(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE799);
  COPY(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE799, zbuilder.zframes);
  sail_fixed_bytes_32 z3zE995;
  {
    if ((z2zE799 == NULL)) goto case_1515;
    struct zexception z2zE800;
    CREATE(zexception)(&z2zE800);
    zInvalidBlock(&z2zE800, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE800);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/updates.sail:376.18-376.54");
    KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE799);
    KILL(zexception)(&z2zE800);
    goto end_block_exception_1517;
    /* unreachable after throw */
    KILL(zexception)(&z2zE800);
    goto finish_match_1513;
  }
case_1515: ;
  {
    /* complete */
    bool z2zE801;
    z2zE801 = zbuilder.zcomplete;
    if (z2zE801) {
      struct zNodeRef z2zE802;
      CREATE(zNodeRef)(&z2zE802);
      z2zE802 = zbuilder.zroot;
      z3zE995 = ztrie_ref_to_root(z2zE802);
      KILL(zNodeRef)(&z2zE802);
    } else {  z3zE995 = zEMPTY_TRIE_ROOT;  }
    goto finish_match_1513;
  }
case_1514: ;
finish_match_1513: ;
  z8zE596 = z3zE995;
  KILL(zz5listz8z5structz0zzTrieBranchFramez9)(&z2zE799);
end_function_1516: ;
  return z8zE596;
end_block_exception_1517: ;

  return fixed_bytes_32_zero();
}

sail_fixed_bytes_32 ztrie_sink_root(struct zTrieItemSink zsink)
{
  sail_fixed_bytes_32 z8zE597;
  struct zoptionzIRTrieItemzK z2zE796;
  CREATE(zoptionzIRTrieItemzK)(&z2zE796);
  z2zE796 = zsink.zpending;
  sail_fixed_bytes_32 z3zE992;
  {
    if (z2zE796.kind != Kind_zSomezIRTrieItemzK) goto case_1510;
    struct zexception z2zE797;
    CREATE(zexception)(&z2zE797);
    zInvalidBlock(&z2zE797, zWitnessDeficient);
    COPY(zexception)(current_exception, z2zE797);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/mpt/updates.sail:387.19-387.55");
    KILL(zoptionzIRTrieItemzK)(&z2zE796);
    KILL(zexception)(&z2zE797);
    goto end_block_exception_1512;
    /* unreachable after throw */
    KILL(zexception)(&z2zE797);
    goto finish_match_1508;
  }
case_1510: ;
  {
    /* complete */
    struct zTrieBuilder z2zE798;
    CREATE(zTrieBuilder)(&z2zE798);
    COPY(zTrieBuilder)(&z2zE798, zsink.zbuilder);
    {
      z3zE992 = ztrie_builder_root(z2zE798);
      if (have_exception) {
        KILL(zoptionzIRTrieItemzK)(&z2zE796);
        KILL(zTrieBuilder)(&z2zE798);
        goto end_block_exception_1512;
      }
    }
    KILL(zTrieBuilder)(&z2zE798);
    goto finish_match_1508;
  }
case_1509: ;
finish_match_1508: ;
  z8zE597 = z3zE992;
  KILL(zoptionzIRTrieItemzK)(&z2zE796);
end_function_1511: ;
  return z8zE597;
end_block_exception_1512: ;

  return fixed_bytes_32_zero();
}

