/* Generated from sail/lib/mpt/trie.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
void zemit_live_updates_under(struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 *z8zE549, struct zTrieItemSink zsink, struct zTrieUpdateCursor zupdates, struct zTriePath zprefix)
{
  struct zoptionzIRTrieUpdatezK z2zE1228;
  CREATE(zoptionzIRTrieUpdatezK)(&z2zE1228);
  COPY(zoptionzIRTrieUpdatezK)(&z2zE1228, zupdates.zpending);
  struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z3zE1306;
  CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1306);
  {
    if (z2zE1228.kind != Kind_zNonezIRTrieUpdatezK) goto case_1748;
    COPY(zTrieItemSink)(&((&z3zE1306)->ztup0), zsink);
    COPY(zTrieUpdateCursor)(&((&z3zE1306)->ztup1), zupdates);
    goto finish_match_1743;
  }
case_1748: ;
  {
    /* complete */
    struct zTrieUpdate zupdate;
    CREATE(zTrieUpdate)(&zupdate);
    COPY(zTrieUpdate)(&zupdate, z2zE1228.variants.zSomezIRTrieUpdatezK);
    bool z2zE1230;
    {
      struct zTriePath z2zE1229;
      z2zE1229 = zupdate.zkey;
      {
        z2zE1230 = zpath_prefix_of(zprefix, z2zE1229);
        if (have_exception) {
          KILL(zoptionzIRTrieUpdatezK)(&z2zE1228);
          KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1306);
          KILL(zTrieUpdate)(&zupdate);
          goto end_block_exception_1750;
        }
      }
    }
    if (z2zE1230) {
      struct zTrieItemSink znext_sink;
      CREATE(zTrieItemSink)(&znext_sink);
      {
        struct zTrieChange z2zE1232;
        CREATE(zTrieChange)(&z2zE1232);
        COPY(zTrieChange)(&z2zE1232, zupdate.zchange);
        struct zTrieItemSink z3zE1309;
        CREATE(zTrieItemSink)(&z3zE1309);
        {
          if (z2zE1232.kind != Kind_zTrieDelete) goto case_1747;
          COPY(zTrieItemSink)(&z3zE1309, zsink);
          goto finish_match_1745;
        }
      case_1747: ;
        {
          /* complete */
          struct zByteSliceFields zvalue;
          zvalue = z2zE1232.variants.zTriePut;
          struct zTrieItem z2zE1234;
          CREATE(zTrieItem)(&z2zE1234);
          {
            struct zTriePath z2zE1233;
            z2zE1233 = zupdate.zkey;
            zitem_leaf(&z2zE1234, z2zE1233, zvalue);
          }
          {
            ztrie_sink_emit(&z3zE1309, zsink, z2zE1234);
            if (have_exception) {
              KILL(zoptionzIRTrieUpdatezK)(&z2zE1228);
              KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1306);
              KILL(zTrieUpdate)(&zupdate);
              KILL(zTrieItemSink)(&znext_sink);
              KILL(zTrieChange)(&z2zE1232);
              KILL(zTrieItemSink)(&z3zE1309);
              KILL(zTrieItem)(&z2zE1234);
              goto end_block_exception_1750;
            }
          }
          KILL(zTrieItem)(&z2zE1234);
          goto finish_match_1745;
        }
      case_1746: ;
      finish_match_1745: ;
        COPY(zTrieItemSink)(&znext_sink, z3zE1309);
        KILL(zTrieItemSink)(&z3zE1309);
        KILL(zTrieChange)(&z2zE1232);
      }
      struct zTrieUpdateCursor z2zE1231;
      CREATE(zTrieUpdateCursor)(&z2zE1231);
      ztrie_updates_advance(&z2zE1231, zupdates);
      {
        zemit_live_updates_under(&z3zE1306, znext_sink, z2zE1231, zprefix);
        if (have_exception) {
          KILL(zoptionzIRTrieUpdatezK)(&z2zE1228);
          KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1306);
          KILL(zTrieUpdate)(&zupdate);
          KILL(zTrieItemSink)(&znext_sink);
          KILL(zTrieUpdateCursor)(&z2zE1231);
          goto end_block_exception_1750;
        }
      }
      KILL(zTrieUpdateCursor)(&z2zE1231);
      KILL(zTrieItemSink)(&znext_sink);
    } else {
      struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z3zE1308;
      CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1308);
      COPY(zTrieItemSink)(&((&z3zE1308)->ztup0), zsink);
      COPY(zTrieUpdateCursor)(&((&z3zE1308)->ztup1), zupdates);
      COPY(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1306, z3zE1308);
      KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1308);
    }
    KILL(zTrieUpdate)(&zupdate);
    goto finish_match_1743;
  }
case_1744: ;
finish_match_1743: ;
  COPY(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)((*(&z8zE549)), z3zE1306);
  KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1306);
  KILL(zoptionzIRTrieUpdatezK)(&z2zE1228);
end_function_1749: ;
  goto end_function_4005;
end_block_exception_1750: ;
  goto end_function_4005;
end_function_4005: ;
}

void zemit_updates_before_child(struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 *z8zE550, struct zTrieItemSink zsink, struct zTrieUpdateCursor zupdates, struct zTriePath zprefix, struct zTriePath zchild)
{
  struct zoptionzIRTrieUpdatezK z2zE1215;
  CREATE(zoptionzIRTrieUpdatezK)(&z2zE1215);
  COPY(zoptionzIRTrieUpdatezK)(&z2zE1215, zupdates.zpending);
  struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z3zE1296;
  CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1296);
  {
    if (z2zE1215.kind != Kind_zNonezIRTrieUpdatezK) goto case_1740;
    COPY(zTrieItemSink)(&((&z3zE1296)->ztup0), zsink);
    COPY(zTrieUpdateCursor)(&((&z3zE1296)->ztup1), zupdates);
    goto finish_match_1735;
  }
case_1740: ;
  {
    /* complete */
    struct zTrieUpdate zupdate;
    CREATE(zTrieUpdate)(&zupdate);
    COPY(zTrieUpdate)(&zupdate, z2zE1215.variants.zSomezIRTrieUpdatezK);
    bool z2zE1223;
    {
      bool z2zE1222;
      {
        bool z2zE1217;
        {
          struct zTriePath z2zE1216;
          z2zE1216 = zupdate.zkey;
          {
            z2zE1217 = zpath_prefix_of(zprefix, z2zE1216);
            if (have_exception) {
              KILL(zoptionzIRTrieUpdatezK)(&z2zE1215);
              KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1296);
              KILL(zTrieUpdate)(&zupdate);
              goto end_block_exception_1742;
            }
          }
        }
        z2zE1222 = not(z2zE1217);
      }
      bool z3zE1298;
      if (z2zE1222) {  z3zE1298 = true;  } else {
        bool z2zE1221;
        {
          struct zTriePath z2zE1218;
          z2zE1218 = zupdate.zkey;
          {
            z2zE1221 = zpath_prefix_of(zchild, z2zE1218);
            if (have_exception) {
              KILL(zoptionzIRTrieUpdatezK)(&z2zE1215);
              KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1296);
              KILL(zTrieUpdate)(&zupdate);
              goto end_block_exception_1742;
            }
          }
        }
        bool z3zE1297;
        if (z2zE1221) {  z3zE1297 = true;  } else {
          bool z2zE1220;
          {
            struct zTriePath z2zE1219;
            z2zE1219 = zupdate.zkey;
            z2zE1220 = zpath_lt(z2zE1219, zchild);
          }
          z3zE1297 = not(z2zE1220);
        }
        z3zE1298 = z3zE1297;
      }
      z2zE1223 = z3zE1298;
    }
    if (z2zE1223) {
      struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z3zE1303;
      CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1303);
      COPY(zTrieItemSink)(&((&z3zE1303)->ztup0), zsink);
      COPY(zTrieUpdateCursor)(&((&z3zE1303)->ztup1), zupdates);
      COPY(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1296, z3zE1303);
      KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1303);
    } else {
      struct zTrieItemSink znext_sink;
      CREATE(zTrieItemSink)(&znext_sink);
      {
        struct zTrieChange z2zE1225;
        CREATE(zTrieChange)(&z2zE1225);
        COPY(zTrieChange)(&z2zE1225, zupdate.zchange);
        struct zTrieItemSink z3zE1300;
        CREATE(zTrieItemSink)(&z3zE1300);
        {
          if (z2zE1225.kind != Kind_zTrieDelete) goto case_1739;
          COPY(zTrieItemSink)(&z3zE1300, zsink);
          goto finish_match_1737;
        }
      case_1739: ;
        {
          /* complete */
          struct zByteSliceFields zvalue;
          zvalue = z2zE1225.variants.zTriePut;
          struct zTrieItem z2zE1227;
          CREATE(zTrieItem)(&z2zE1227);
          {
            struct zTriePath z2zE1226;
            z2zE1226 = zupdate.zkey;
            zitem_leaf(&z2zE1227, z2zE1226, zvalue);
          }
          {
            ztrie_sink_emit(&z3zE1300, zsink, z2zE1227);
            if (have_exception) {
              KILL(zoptionzIRTrieUpdatezK)(&z2zE1215);
              KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1296);
              KILL(zTrieUpdate)(&zupdate);
              KILL(zTrieItemSink)(&znext_sink);
              KILL(zTrieChange)(&z2zE1225);
              KILL(zTrieItemSink)(&z3zE1300);
              KILL(zTrieItem)(&z2zE1227);
              goto end_block_exception_1742;
            }
          }
          KILL(zTrieItem)(&z2zE1227);
          goto finish_match_1737;
        }
      case_1738: ;
      finish_match_1737: ;
        COPY(zTrieItemSink)(&znext_sink, z3zE1300);
        KILL(zTrieItemSink)(&z3zE1300);
        KILL(zTrieChange)(&z2zE1225);
      }
      struct zTrieUpdateCursor z2zE1224;
      CREATE(zTrieUpdateCursor)(&z2zE1224);
      ztrie_updates_advance(&z2zE1224, zupdates);
      {
        zemit_updates_before_child(&z3zE1296, znext_sink, z2zE1224, zprefix, zchild);
        if (have_exception) {
          KILL(zoptionzIRTrieUpdatezK)(&z2zE1215);
          KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1296);
          KILL(zTrieUpdate)(&zupdate);
          KILL(zTrieItemSink)(&znext_sink);
          KILL(zTrieUpdateCursor)(&z2zE1224);
          goto end_block_exception_1742;
        }
      }
      KILL(zTrieUpdateCursor)(&z2zE1224);
      KILL(zTrieItemSink)(&znext_sink);
    }
    KILL(zTrieUpdate)(&zupdate);
    goto finish_match_1735;
  }
case_1736: ;
finish_match_1735: ;
  COPY(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)((*(&z8zE550)), z3zE1296);
  KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1296);
  KILL(zoptionzIRTrieUpdatezK)(&z2zE1215);
end_function_1741: ;
  goto end_function_4004;
end_block_exception_1742: ;
  goto end_function_4004;
end_function_4004: ;
}

void zemit_leaf_overlay(struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 *z8zE551, struct zTrieItemSink zsink, struct zTrieUpdateCursor zupdates, struct zTriePath zprefix, struct zTriePath zkey, struct zByteSliceFields zvalue)
{
  struct zoptionzIRTrieUpdatezK z2zE1194;
  CREATE(zoptionzIRTrieUpdatezK)(&z2zE1194);
  COPY(zoptionzIRTrieUpdatezK)(&z2zE1194, zupdates.zpending);
  struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z3zE1283;
  CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1283);
  {
    if (z2zE1194.kind != Kind_zNonezIRTrieUpdatezK) goto case_1732;
    struct zTrieItemSink z2zE1196;
    CREATE(zTrieItemSink)(&z2zE1196);
    {
      struct zTrieItem z2zE1195;
      CREATE(zTrieItem)(&z2zE1195);
      zitem_leaf(&z2zE1195, zkey, zvalue);
      {
        ztrie_sink_emit(&z2zE1196, zsink, z2zE1195);
        if (have_exception) {
          KILL(zoptionzIRTrieUpdatezK)(&z2zE1194);
          KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1283);
          KILL(zTrieItemSink)(&z2zE1196);
          KILL(zTrieItem)(&z2zE1195);
          goto end_block_exception_1734;
        }
      }
      KILL(zTrieItem)(&z2zE1195);
    }
    COPY(zTrieItemSink)(&((&z3zE1283)->ztup0), z2zE1196);
    COPY(zTrieUpdateCursor)(&((&z3zE1283)->ztup1), zupdates);
    KILL(zTrieItemSink)(&z2zE1196);
    goto finish_match_1724;
  }
case_1732: ;
  {
    /* complete */
    struct zTrieUpdate zupdate;
    CREATE(zTrieUpdate)(&zupdate);
    COPY(zTrieUpdate)(&zupdate, z2zE1194.variants.zSomezIRTrieUpdatezK);
    bool z2zE1199;
    {
      bool z2zE1198;
      {
        struct zTriePath z2zE1197;
        z2zE1197 = zupdate.zkey;
        {
          z2zE1198 = zpath_prefix_of(zprefix, z2zE1197);
          if (have_exception) {
            KILL(zoptionzIRTrieUpdatezK)(&z2zE1194);
            KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1283);
            KILL(zTrieUpdate)(&zupdate);
            goto end_block_exception_1734;
          }
        }
      }
      z2zE1199 = not(z2zE1198);
    }
    if (z2zE1199) {
      struct zTrieItemSink z2zE1201;
      CREATE(zTrieItemSink)(&z2zE1201);
      {
        struct zTrieItem z2zE1200;
        CREATE(zTrieItem)(&z2zE1200);
        zitem_leaf(&z2zE1200, zkey, zvalue);
        {
          ztrie_sink_emit(&z2zE1201, zsink, z2zE1200);
          if (have_exception) {
            KILL(zoptionzIRTrieUpdatezK)(&z2zE1194);
            KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1283);
            KILL(zTrieUpdate)(&zupdate);
            KILL(zTrieItemSink)(&z2zE1201);
            KILL(zTrieItem)(&z2zE1200);
            goto end_block_exception_1734;
          }
        }
        KILL(zTrieItem)(&z2zE1200);
      }
      struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z3zE1291;
      CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1291);
      COPY(zTrieItemSink)(&((&z3zE1291)->ztup0), z2zE1201);
      COPY(zTrieUpdateCursor)(&((&z3zE1291)->ztup1), zupdates);
      COPY(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1283, z3zE1291);
      KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1291);
      KILL(zTrieItemSink)(&z2zE1201);
    } else {
      bool z2zE1203;
      {
        struct zTriePath z2zE1202;
        z2zE1202 = zupdate.zkey;
        z2zE1203 = zpath_eq(z2zE1202, zkey);
      }
      if (z2zE1203) {
        struct zTrieItemSink zupdated_sink;
        CREATE(zTrieItemSink)(&zupdated_sink);
        {
          struct zTrieChange z2zE1205;
          CREATE(zTrieChange)(&z2zE1205);
          COPY(zTrieChange)(&z2zE1205, zupdate.zchange);
          struct zTrieItemSink z3zE1288;
          CREATE(zTrieItemSink)(&z3zE1288);
          {
            if (z2zE1205.kind != Kind_zTrieDelete) goto case_1731;
            COPY(zTrieItemSink)(&z3zE1288, zsink);
            goto finish_match_1729;
          }
        case_1731: ;
          {
            /* complete */
            struct zByteSliceFields zupdated;
            zupdated = z2zE1205.variants.zTriePut;
            struct zTrieItem z2zE1206;
            CREATE(zTrieItem)(&z2zE1206);
            zitem_leaf(&z2zE1206, zkey, zupdated);
            {
              ztrie_sink_emit(&z3zE1288, zsink, z2zE1206);
              if (have_exception) {
                KILL(zoptionzIRTrieUpdatezK)(&z2zE1194);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1283);
                KILL(zTrieUpdate)(&zupdate);
                KILL(zTrieItemSink)(&zupdated_sink);
                KILL(zTrieChange)(&z2zE1205);
                KILL(zTrieItemSink)(&z3zE1288);
                KILL(zTrieItem)(&z2zE1206);
                goto end_block_exception_1734;
              }
            }
            KILL(zTrieItem)(&z2zE1206);
            goto finish_match_1729;
          }
        case_1730: ;
        finish_match_1729: ;
          COPY(zTrieItemSink)(&zupdated_sink, z3zE1288);
          KILL(zTrieItemSink)(&z3zE1288);
          KILL(zTrieChange)(&z2zE1205);
        }
        struct zTrieUpdateCursor z2zE1204;
        CREATE(zTrieUpdateCursor)(&z2zE1204);
        ztrie_updates_advance(&z2zE1204, zupdates);
        {
          zemit_live_updates_under(&z3zE1283, zupdated_sink, z2zE1204, zprefix);
          if (have_exception) {
            KILL(zoptionzIRTrieUpdatezK)(&z2zE1194);
            KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1283);
            KILL(zTrieUpdate)(&zupdate);
            KILL(zTrieItemSink)(&zupdated_sink);
            KILL(zTrieUpdateCursor)(&z2zE1204);
            goto end_block_exception_1734;
          }
        }
        KILL(zTrieUpdateCursor)(&z2zE1204);
        KILL(zTrieItemSink)(&zupdated_sink);
      } else {
        bool z2zE1208;
        {
          struct zTriePath z2zE1207;
          z2zE1207 = zupdate.zkey;
          z2zE1208 = zpath_lt(zkey, z2zE1207);
        }
        if (z2zE1208) {
          struct zTrieItemSink z2zE1210;
          CREATE(zTrieItemSink)(&z2zE1210);
          {
            struct zTrieItem z2zE1209;
            CREATE(zTrieItem)(&z2zE1209);
            zitem_leaf(&z2zE1209, zkey, zvalue);
            {
              ztrie_sink_emit(&z2zE1210, zsink, z2zE1209);
              if (have_exception) {
                KILL(zoptionzIRTrieUpdatezK)(&z2zE1194);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1283);
                KILL(zTrieUpdate)(&zupdate);
                KILL(zTrieItemSink)(&z2zE1210);
                KILL(zTrieItem)(&z2zE1209);
                goto end_block_exception_1734;
              }
            }
            KILL(zTrieItem)(&z2zE1209);
          }
          {
            zemit_live_updates_under(&z3zE1283, z2zE1210, zupdates, zprefix);
            if (have_exception) {
              KILL(zoptionzIRTrieUpdatezK)(&z2zE1194);
              KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1283);
              KILL(zTrieUpdate)(&zupdate);
              KILL(zTrieItemSink)(&z2zE1210);
              goto end_block_exception_1734;
            }
          }
          KILL(zTrieItemSink)(&z2zE1210);
        } else {
          struct zTrieItemSink z3zE1294;
          CREATE(zTrieItemSink)(&z3zE1294);
          {
            struct zTrieChange z2zE1212;
            CREATE(zTrieChange)(&z2zE1212);
            COPY(zTrieChange)(&z2zE1212, zupdate.zchange);
            struct zTrieItemSink z3zE1285;
            CREATE(zTrieItemSink)(&z3zE1285);
            {
              if (z2zE1212.kind != Kind_zTrieDelete) goto case_1728;
              COPY(zTrieItemSink)(&z3zE1285, zsink);
              goto finish_match_1726;
            }
          case_1728: ;
            {
              /* complete */
              struct zByteSliceFields z3zE1295;
              z3zE1295 = z2zE1212.variants.zTriePut;
              struct zTrieItem z2zE1214;
              CREATE(zTrieItem)(&z2zE1214);
              {
                struct zTriePath z2zE1213;
                z2zE1213 = zupdate.zkey;
                zitem_leaf(&z2zE1214, z2zE1213, z3zE1295);
              }
              {
                ztrie_sink_emit(&z3zE1285, zsink, z2zE1214);
                if (have_exception) {
                  KILL(zoptionzIRTrieUpdatezK)(&z2zE1194);
                  KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1283);
                  KILL(zTrieUpdate)(&zupdate);
                  KILL(zTrieItemSink)(&z3zE1294);
                  KILL(zTrieChange)(&z2zE1212);
                  KILL(zTrieItemSink)(&z3zE1285);
                  KILL(zTrieItem)(&z2zE1214);
                  goto end_block_exception_1734;
                }
              }
              KILL(zTrieItem)(&z2zE1214);
              goto finish_match_1726;
            }
          case_1727: ;
          finish_match_1726: ;
            COPY(zTrieItemSink)(&z3zE1294, z3zE1285);
            KILL(zTrieItemSink)(&z3zE1285);
            KILL(zTrieChange)(&z2zE1212);
          }
          struct zTrieUpdateCursor z2zE1211;
          CREATE(zTrieUpdateCursor)(&z2zE1211);
          ztrie_updates_advance(&z2zE1211, zupdates);
          {
            zemit_leaf_overlay(&z3zE1283, z3zE1294, z2zE1211, zprefix, zkey, zvalue);
            if (have_exception) {
              KILL(zoptionzIRTrieUpdatezK)(&z2zE1194);
              KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1283);
              KILL(zTrieUpdate)(&zupdate);
              KILL(zTrieItemSink)(&z3zE1294);
              KILL(zTrieUpdateCursor)(&z2zE1211);
              goto end_block_exception_1734;
            }
          }
          KILL(zTrieUpdateCursor)(&z2zE1211);
          KILL(zTrieItemSink)(&z3zE1294);
        }
      }
    }
    KILL(zTrieUpdate)(&zupdate);
    goto finish_match_1724;
  }
case_1725: ;
finish_match_1724: ;
  COPY(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)((*(&z8zE551)), z3zE1283);
  KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1283);
  KILL(zoptionzIRTrieUpdatezK)(&z2zE1194);
end_function_1733: ;
  goto end_function_4003;
end_block_exception_1734: ;
  goto end_function_4003;
end_function_4003: ;
}

void zwitness_emit(struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 *z8zE552, struct zByteSliceFields znode, struct zTriePath zprefix, struct zTrieUpdateCursor zupdates, struct zTrieItemSink zsink, uint64_t zcursor)
{
  bool z2zE1173;
  {
    uint64_t z2zE1172;
    z2zE1172 = znode.zlen;
    z2zE1173 = (z2zE1172 == UINT64_C(0));
  }
  if (z2zE1173) {
    {
      zemit_live_updates_under((*(&z8zE552)), zsink, zupdates, zprefix);
      if (have_exception) {  goto end_block_exception_1723;  }
    }
  } else {
    struct zTrieNode z2zE1174;
    CREATE(zTrieNode)(&z2zE1174);
    {
      zdecode_trie_node(&z2zE1174, znode);
      if (have_exception) {
        KILL(zTrieNode)(&z2zE1174);
        goto end_block_exception_1723;
      }
    }
    struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z3zE1254;
    CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
    {
      if (z2zE1174.kind != Kind_zLeafNode) goto case_1721;
      struct zTriePath zpath;
      zpath = z2zE1174.variants.zLeafNode.ztup0;
      struct zByteSliceFields zvalue;
      zvalue = z2zE1174.variants.zLeafNode.ztup1;
      struct zTriePath zkey;
      {
        zkey = zpath_concat(zprefix, zpath);
        if (have_exception) {
          KILL(zTrieNode)(&z2zE1174);
          KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
          goto end_block_exception_1723;
        }
      }
      {
        zemit_leaf_overlay(&z3zE1254, zsink, zupdates, zprefix, zkey, zvalue);
        if (have_exception) {
          KILL(zTrieNode)(&z2zE1174);
          KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
          goto end_block_exception_1723;
        }
      }
      goto finish_match_1707;
    }
  case_1721: ;
    {
      if (z2zE1174.kind != Kind_zExtensionNode) goto case_1716;
      struct zTriePath z3zE1278;
      z3zE1278 = z2zE1174.variants.zExtensionNode.ztup0;
      struct zNodeRef zchildref;
      CREATE(zNodeRef)(&zchildref);
      COPY(zNodeRef)(&zchildref, z2zE1174.variants.zExtensionNode.ztup1);
      uint64_t zextension_len;
      zextension_len = zpath_len(z3zE1278);
      uint64_t znext_cursor;
      {    znext_cursor = (zcursor + zextension_len);
      }
      bool z2zE1176;
      {
        bool z2zE1175;
        z2zE1175 = (zextension_len == UINT64_C(0));
        bool z3zE1270;
        if (z2zE1175) {  z3zE1270 = true;  } else {  z3zE1270 = (UINT64_C(64) < znext_cursor);  }
        z2zE1176 = z3zE1270;
      }
      if (z2zE1176) {
        struct zexception z2zE1177;
        CREATE(zexception)(&z2zE1177);
        zInvalidBlock(&z2zE1177, zWitnessDeficient);
        COPY(zexception)(current_exception, z2zE1177);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/lib/mpt/trie.sail:143.20-143.56");
        KILL(zTrieNode)(&z2zE1174);
        KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
        KILL(zNodeRef)(&zchildref);
        KILL(zexception)(&z2zE1177);
        goto end_block_exception_1723;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1177);
      } else {
        struct zTriePath zchild_prefix;
        {
          zchild_prefix = zpath_concat(zprefix, z3zE1278);
          if (have_exception) {
            KILL(zTrieNode)(&z2zE1174);
            KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
            KILL(zNodeRef)(&zchildref);
            goto end_block_exception_1723;
          }
        }
        struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z2zE1178;
        CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1178);
        {
          zemit_updates_before_child(&z2zE1178, zsink, zupdates, zprefix, zchild_prefix);
          if (have_exception) {
            KILL(zTrieNode)(&z2zE1174);
            KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
            KILL(zNodeRef)(&zchildref);
            KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1178);
            goto end_block_exception_1723;
          }
        }
        struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z3zE1272;
        CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1272);
        {
          struct zTrieItemSink zbefore_sink;
          CREATE(zTrieItemSink)(&zbefore_sink);
          COPY(zTrieItemSink)(&zbefore_sink, z2zE1178.ztup0);
          struct zTrieUpdateCursor zchild_updates;
          CREATE(zTrieUpdateCursor)(&zchild_updates);
          COPY(zTrieUpdateCursor)(&zchild_updates, z2zE1178.ztup1);
          struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z2zE1184;
          CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1184);
          {
            bool z2zE1179;
            {
              z2zE1179 = znext_update_under(zchild_updates, zchild_prefix);
              if (have_exception) {
                KILL(zTrieNode)(&z2zE1174);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
                KILL(zNodeRef)(&zchildref);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1178);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1272);
                KILL(zTrieItemSink)(&zbefore_sink);
                KILL(zTrieUpdateCursor)(&zchild_updates);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1184);
                goto end_block_exception_1723;
              }
            }
            if (z2zE1179) {
              struct zByteSliceFields zchild;
              {
                zchild = zresolve_ref(zchildref);
                if (have_exception) {
                  KILL(zTrieNode)(&z2zE1174);
                  KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
                  KILL(zNodeRef)(&zchildref);
                  KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1178);
                  KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1272);
                  KILL(zTrieItemSink)(&zbefore_sink);
                  KILL(zTrieUpdateCursor)(&zchild_updates);
                  KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1184);
                  goto end_block_exception_1723;
                }
              }
              bool z2zE1181;
              {
                uint64_t z2zE1180;
                z2zE1180 = zchild.zlen;
                z2zE1181 = (z2zE1180 == UINT64_C(0));
              }
              if (z2zE1181) {
                {
                  zemit_live_updates_under(&z2zE1184, zbefore_sink, zchild_updates, zchild_prefix);
                  if (have_exception) {
                    KILL(zTrieNode)(&z2zE1174);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
                    KILL(zNodeRef)(&zchildref);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1178);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1272);
                    KILL(zTrieItemSink)(&zbefore_sink);
                    KILL(zTrieUpdateCursor)(&zchild_updates);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1184);
                    goto end_block_exception_1723;
                  }
                }
              } else {
                {
                  zwitness_emit(&z2zE1184, zchild, zchild_prefix, zchild_updates, zbefore_sink, znext_cursor);
                  if (have_exception) {
                    KILL(zTrieNode)(&z2zE1174);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
                    KILL(zNodeRef)(&zchildref);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1178);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1272);
                    KILL(zTrieItemSink)(&zbefore_sink);
                    KILL(zTrieUpdateCursor)(&zchild_updates);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1184);
                    goto end_block_exception_1723;
                  }
                }
              }
            } else {
              struct zTrieItemSink z2zE1183;
              CREATE(zTrieItemSink)(&z2zE1183);
              {
                struct zTrieItem z2zE1182;
                CREATE(zTrieItem)(&z2zE1182);
                zitem_branch(&z2zE1182, zchild_prefix, zchildref);
                {
                  ztrie_sink_emit(&z2zE1183, zbefore_sink, z2zE1182);
                  if (have_exception) {
                    KILL(zTrieNode)(&z2zE1174);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
                    KILL(zNodeRef)(&zchildref);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1178);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1272);
                    KILL(zTrieItemSink)(&zbefore_sink);
                    KILL(zTrieUpdateCursor)(&zchild_updates);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1184);
                    KILL(zTrieItemSink)(&z2zE1183);
                    KILL(zTrieItem)(&z2zE1182);
                    goto end_block_exception_1723;
                  }
                }
                KILL(zTrieItem)(&z2zE1182);
              }
              struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z3zE1273;
              CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1273);
              COPY(zTrieItemSink)(&((&z3zE1273)->ztup0), z2zE1183);
              COPY(zTrieUpdateCursor)(&((&z3zE1273)->ztup1), zchild_updates);
              COPY(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1184, z3zE1273);
              KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1273);
              KILL(zTrieItemSink)(&z2zE1183);
            }
          }
          struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z3zE1274;
          CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1274);
          {
            struct zTrieItemSink zchild_sink;
            CREATE(zTrieItemSink)(&zchild_sink);
            COPY(zTrieItemSink)(&zchild_sink, z2zE1184.ztup0);
            struct zTrieUpdateCursor zlater_updates;
            CREATE(zTrieUpdateCursor)(&zlater_updates);
            COPY(zTrieUpdateCursor)(&zlater_updates, z2zE1184.ztup1);
            {
              zemit_live_updates_under(&z3zE1274, zchild_sink, zlater_updates, zprefix);
              if (have_exception) {
                KILL(zTrieNode)(&z2zE1174);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
                KILL(zNodeRef)(&zchildref);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1178);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1272);
                KILL(zTrieItemSink)(&zbefore_sink);
                KILL(zTrieUpdateCursor)(&zchild_updates);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1184);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1274);
                KILL(zTrieItemSink)(&zchild_sink);
                KILL(zTrieUpdateCursor)(&zlater_updates);
                goto end_block_exception_1723;
              }
            }
            KILL(zTrieUpdateCursor)(&zlater_updates);
            KILL(zTrieItemSink)(&zchild_sink);
            goto finish_match_1719;
          }
        case_1720: ;
          sail_match_failure("witness_emit");
        finish_match_1719: ;
          COPY(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1272, z3zE1274);
          KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1274);
          KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1184);
          KILL(zTrieUpdateCursor)(&zchild_updates);
          KILL(zTrieItemSink)(&zbefore_sink);
          goto finish_match_1717;
        }
      case_1718: ;
        sail_match_failure("witness_emit");
      finish_match_1717: ;
        COPY(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254, z3zE1272);
        KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1272);
        KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1178);
      }
      KILL(zNodeRef)(&zchildref);
      goto finish_match_1707;
    }
  case_1716: ;
    {
      /* complete */
      zz5vecz8z5unionz0zzNodeRefz9 zchildren;
      CREATE(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren);
      COPY(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren, z2zE1174.variants.zBranchNode.ztup0);
      struct zByteSliceFields z3zE1279;
      z3zE1279 = z2zE1174.variants.zBranchNode.ztup1;
      bool z2zE1187;
      {
        bool z2zE1186;
        {
          uint64_t z2zE1185;
          z2zE1185 = z3zE1279.zlen;
          z2zE1186 = (z2zE1185 != UINT64_C(0));
        }
        bool z3zE1255;
        if (z2zE1186) {  z3zE1255 = true;  } else {  z3zE1255 = (!(zcursor < UINT64_C(64)));  }
        z2zE1187 = z3zE1255;
      }
      if (z2zE1187) {
        struct zexception z2zE1188;
        CREATE(zexception)(&z2zE1188);
        zInvalidBlock(&z2zE1188, zWitnessDeficient);
        COPY(zexception)(current_exception, z2zE1188);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/lib/mpt/trie.sail:165.20-165.56");
        KILL(zTrieNode)(&z2zE1174);
        KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
        KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren);
        KILL(zexception)(&z2zE1188);
        goto end_block_exception_1723;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1188);
      } else {
        uint64_t z3zE1280;
        {    z3zE1280 = (zcursor + UINT64_C(1));
        }
        struct zTrieItemSink zcurrent_sink;
        CREATE(zTrieItemSink)(&zcurrent_sink);
        COPY(zTrieItemSink)(&zcurrent_sink, zsink);
        struct zTrieUpdateCursor zremaining;
        CREATE(zTrieUpdateCursor)(&zremaining);
        COPY(zTrieUpdateCursor)(&zremaining, zupdates);
        uint64_t znib;
        znib = UINT64_C(0x0);
        int64_t z3zE1257;
        {    z3zE1257 = (int64_t)(UINT64_C(0));
        }
        int64_t z3zE1258;
        {    z3zE1258 = (int64_t)(UINT64_C(15));
        }
        int64_t z3zE1259;
        {    z3zE1259 = (int64_t)(UINT64_C(1));
        }
        {
          int64_t zi;
          zi = z3zE1257;
          unit z3zE1267;
        for_start_1709: ;
          {
            if ((z3zE1258 < zi)) goto for_end_1710;
            struct zTriePath z3zE1281;
            {
              struct zTriePath z2zE1193;
              {
                z2zE1193 = zpath_single(znib);
                if (have_exception) {
                  KILL(zTrieNode)(&z2zE1174);
                  KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
                  KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren);
                  KILL(zTrieItemSink)(&zcurrent_sink);
                  KILL(zTrieUpdateCursor)(&zremaining);
                  goto end_block_exception_1723;
                }
              }
              {
                z3zE1281 = zpath_concat(zprefix, z2zE1193);
                if (have_exception) {
                  KILL(zTrieNode)(&z2zE1174);
                  KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
                  KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren);
                  KILL(zTrieItemSink)(&zcurrent_sink);
                  KILL(zTrieUpdateCursor)(&zremaining);
                  goto end_block_exception_1723;
                }
              }
            }
            struct zNodeRef z3zE1282;
            CREATE(zNodeRef)(&z3zE1282);
            fast_vector_access_zz5vecz8z5unionz0zzNodeRefz9(&z3zE1282, zchildren, zi);
            bool zpresent;
            {
              bool z3zE1260;
              {
                if (z3zE1282.kind != Kind_zEmptyRef) goto case_1713;
                z3zE1260 = false;
                goto finish_match_1711;
              }
            case_1713: ;
              {
                z3zE1260 = true;
                goto finish_match_1711;
              }
            case_1712: ;
            finish_match_1711: ;
              zpresent = z3zE1260;
            }
            bool z2zE1189;
            {
              z2zE1189 = znext_update_under(zremaining, z3zE1281);
              if (have_exception) {
                KILL(zTrieNode)(&z2zE1174);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
                KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren);
                KILL(zTrieItemSink)(&zcurrent_sink);
                KILL(zTrieUpdateCursor)(&zremaining);
                KILL(zNodeRef)(&z3zE1282);
                goto end_block_exception_1723;
              }
            }
            unit z3zE1263;
            if (z2zE1189) {
              struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z2zE1191;
              CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1191);
              if (zpresent) {
                struct zByteSliceFields z2zE1190;
                {
                  z2zE1190 = zresolve_ref(z3zE1282);
                  if (have_exception) {
                    KILL(zTrieNode)(&z2zE1174);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
                    KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren);
                    KILL(zTrieItemSink)(&zcurrent_sink);
                    KILL(zTrieUpdateCursor)(&zremaining);
                    KILL(zNodeRef)(&z3zE1282);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1191);
                    goto end_block_exception_1723;
                  }
                }
                {
                  zwitness_emit(&z2zE1191, z2zE1190, z3zE1281, zremaining, zcurrent_sink, z3zE1280);
                  if (have_exception) {
                    KILL(zTrieNode)(&z2zE1174);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
                    KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren);
                    KILL(zTrieItemSink)(&zcurrent_sink);
                    KILL(zTrieUpdateCursor)(&zremaining);
                    KILL(zNodeRef)(&z3zE1282);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1191);
                    goto end_block_exception_1723;
                  }
                }
              } else {
                {
                  zemit_live_updates_under(&z2zE1191, zcurrent_sink, zremaining, z3zE1281);
                  if (have_exception) {
                    KILL(zTrieNode)(&z2zE1174);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
                    KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren);
                    KILL(zTrieItemSink)(&zcurrent_sink);
                    KILL(zTrieUpdateCursor)(&zremaining);
                    KILL(zNodeRef)(&z3zE1282);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1191);
                    goto end_block_exception_1723;
                  }
                }
              }
              unit z3zE1264;
              {
                struct zTrieItemSink znext_sink;
                CREATE(zTrieItemSink)(&znext_sink);
                COPY(zTrieItemSink)(&znext_sink, z2zE1191.ztup0);
                struct zTrieUpdateCursor znext_updates;
                CREATE(zTrieUpdateCursor)(&znext_updates);
                COPY(zTrieUpdateCursor)(&znext_updates, z2zE1191.ztup1);
                COPY(zTrieItemSink)(&zcurrent_sink, znext_sink);
                unit z3zE1265;
                z3zE1265 = UNIT;
                COPY(zTrieUpdateCursor)(&zremaining, znext_updates);
                z3zE1264 = UNIT;
                KILL(zTrieUpdateCursor)(&znext_updates);
                KILL(zTrieItemSink)(&znext_sink);
                goto finish_match_1714;
              }
            case_1715: ;
              sail_match_failure("witness_emit");
            finish_match_1714: ;
              z3zE1263 = z3zE1264;
              KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1191);
            } else if (zpresent) {
              struct zTrieItem z2zE1192;
              CREATE(zTrieItem)(&z2zE1192);
              zitem_subtree(&z2zE1192, z3zE1281, z3zE1282);
              {
                ztrie_sink_emit(&zcurrent_sink, zcurrent_sink, z2zE1192);
                if (have_exception) {
                  KILL(zTrieNode)(&z2zE1174);
                  KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
                  KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren);
                  KILL(zTrieItemSink)(&zcurrent_sink);
                  KILL(zTrieUpdateCursor)(&zremaining);
                  KILL(zNodeRef)(&z3zE1282);
                  KILL(zTrieItem)(&z2zE1192);
                  goto end_block_exception_1723;
                }
              }
              z3zE1263 = UNIT;
              KILL(zTrieItem)(&z2zE1192);
            } else {  z3zE1263 = UNIT;  }
            znib = ((znib + UINT64_C(0x1)) & UINT64_C(0xF));
            z3zE1267 = UNIT;
            KILL(zNodeRef)(&z3zE1282);
            zi = (zi + z3zE1259);
            goto for_start_1709;
          }
        for_end_1710: ;
        }
        unit z3zE1268;
        z3zE1268 = UNIT;
        struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z3zE1269;
        CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1269);
        COPY(zTrieItemSink)(&((&z3zE1269)->ztup0), zcurrent_sink);
        COPY(zTrieUpdateCursor)(&((&z3zE1269)->ztup1), zremaining);
        COPY(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254, z3zE1269);
        KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1269);
        KILL(zTrieUpdateCursor)(&zremaining);
        KILL(zTrieItemSink)(&zcurrent_sink);
      }
      KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren);
      goto finish_match_1707;
    }
  case_1708: ;
  finish_match_1707: ;
    COPY(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)((*(&z8zE552)), z3zE1254);
    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
    KILL(zTrieNode)(&z2zE1174);
  }
end_function_1722: ;
  goto end_function_4002;
end_block_exception_1723: ;
  goto end_function_4002;
end_function_4002: ;
}

struct zTrieRootResult ztrie_root_cursor(sail_fixed_bytes_32 zbase_root, struct zTrieUpdateCursor zupdates)
{
  struct zTrieRootResult z8zE553;
  bool z2zE1162;
  z2zE1162 = zupdates_empty(zupdates);
  if (z2zE1162) {
    struct zTrieRootResult z3zE1253;
    z3zE1253.zchanged = false;
    z3zE1253.zroot = zbase_root;
    z8zE553 = z3zE1253;
  } else {
    struct zTrieItemSink zsink;
    CREATE(zTrieItemSink)(&zsink);
    ztrie_sink_empty(&zsink, UNIT);
    struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z2zE1167;
    CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1167);
    {
      bool z2zE1163;
      z2zE1163 = eq_fixed_bytes_32(zbase_root, zEMPTY_TRIE_ROOT);
      if (z2zE1163) {
        struct zTriePath z3zE1248;
        zz5vecz8z5bv8z9 z3zE1249;
        CREATE(zz5vecz8z5bv8z9)(&z3zE1249);
        internal_vector_init_zz5vecz8z5bv8z9(&z3zE1249, INT64_C(32));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1249, z3zE1249, INT64_C(0), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1249, z3zE1249, INT64_C(1), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1249, z3zE1249, INT64_C(2), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1249, z3zE1249, INT64_C(3), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1249, z3zE1249, INT64_C(4), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1249, z3zE1249, INT64_C(5), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1249, z3zE1249, INT64_C(6), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1249, z3zE1249, INT64_C(7), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1249, z3zE1249, INT64_C(8), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1249, z3zE1249, INT64_C(9), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1249, z3zE1249, INT64_C(10), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1249, z3zE1249, INT64_C(11), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1249, z3zE1249, INT64_C(12), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1249, z3zE1249, INT64_C(13), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1249, z3zE1249, INT64_C(14), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1249, z3zE1249, INT64_C(15), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1249, z3zE1249, INT64_C(16), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1249, z3zE1249, INT64_C(17), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1249, z3zE1249, INT64_C(18), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1249, z3zE1249, INT64_C(19), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1249, z3zE1249, INT64_C(20), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1249, z3zE1249, INT64_C(21), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1249, z3zE1249, INT64_C(22), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1249, z3zE1249, INT64_C(23), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1249, z3zE1249, INT64_C(24), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1249, z3zE1249, INT64_C(25), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1249, z3zE1249, INT64_C(26), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1249, z3zE1249, INT64_C(27), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1249, z3zE1249, INT64_C(28), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1249, z3zE1249, INT64_C(29), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1249, z3zE1249, INT64_C(30), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE1249, z3zE1249, INT64_C(31), UINT64_C(0x00));
        for (size_t z8zE1005 = 0; z8zE1005 < 32; ++z8zE1005) {
          z3zE1248.zdata.bytes[z8zE1005] = (uint8_t)(z3zE1249.data[z8zE1005] & UINT64_C(0xff));
        }
        KILL(zz5vecz8z5bv8z9)(&z3zE1249);
        z3zE1248.zlen = UINT64_C(0);
        {
          zemit_live_updates_under(&z2zE1167, zsink, zupdates, z3zE1248);
          if (have_exception) {
            KILL(zTrieItemSink)(&zsink);
            KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1167);
            goto end_block_exception_1706;
          }
        }
      } else {
        struct zByteSliceFields znode;
        znode = znode_db_lookup(zbase_root);
        bool z2zE1165;
        {
          uint64_t z2zE1164;
          z2zE1164 = znode.zlen;
          z2zE1165 = (z2zE1164 == UINT64_C(0));
        }
        if (z2zE1165) {
          struct zexception z2zE1166;
          CREATE(zexception)(&z2zE1166);
          zInvalidBlock(&z2zE1166, zWitnessDeficient);
          COPY(zexception)(current_exception, z2zE1166);
          have_exception = true;
          COPY(sail_string)(throw_location, "sail/lib/mpt/trie.sail:238.20-238.56");
          KILL(zTrieItemSink)(&zsink);
          KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1167);
          KILL(zexception)(&z2zE1166);
          goto end_block_exception_1706;
          /* unreachable after throw */
          KILL(zexception)(&z2zE1166);
        } else {
          struct zTriePath z3zE1246;
          zz5vecz8z5bv8z9 z3zE1247;
          CREATE(zz5vecz8z5bv8z9)(&z3zE1247);
          internal_vector_init_zz5vecz8z5bv8z9(&z3zE1247, INT64_C(32));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1247, z3zE1247, INT64_C(0), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1247, z3zE1247, INT64_C(1), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1247, z3zE1247, INT64_C(2), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1247, z3zE1247, INT64_C(3), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1247, z3zE1247, INT64_C(4), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1247, z3zE1247, INT64_C(5), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1247, z3zE1247, INT64_C(6), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1247, z3zE1247, INT64_C(7), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1247, z3zE1247, INT64_C(8), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1247, z3zE1247, INT64_C(9), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1247, z3zE1247, INT64_C(10), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1247, z3zE1247, INT64_C(11), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1247, z3zE1247, INT64_C(12), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1247, z3zE1247, INT64_C(13), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1247, z3zE1247, INT64_C(14), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1247, z3zE1247, INT64_C(15), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1247, z3zE1247, INT64_C(16), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1247, z3zE1247, INT64_C(17), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1247, z3zE1247, INT64_C(18), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1247, z3zE1247, INT64_C(19), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1247, z3zE1247, INT64_C(20), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1247, z3zE1247, INT64_C(21), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1247, z3zE1247, INT64_C(22), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1247, z3zE1247, INT64_C(23), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1247, z3zE1247, INT64_C(24), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1247, z3zE1247, INT64_C(25), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1247, z3zE1247, INT64_C(26), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1247, z3zE1247, INT64_C(27), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1247, z3zE1247, INT64_C(28), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1247, z3zE1247, INT64_C(29), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1247, z3zE1247, INT64_C(30), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1247, z3zE1247, INT64_C(31), UINT64_C(0x00));
          for (size_t z8zE1004 = 0; z8zE1004 < 32; ++z8zE1004) {
            z3zE1246.zdata.bytes[z8zE1004] = (uint8_t)(z3zE1247.data[z8zE1004] & UINT64_C(0xff));
          }
          KILL(zz5vecz8z5bv8z9)(&z3zE1247);
          z3zE1246.zlen = UINT64_C(0);
          {
            zwitness_emit(&z2zE1167, znode, z3zE1246, zupdates, zsink, UINT64_C(0));
            if (have_exception) {
              KILL(zTrieItemSink)(&zsink);
              KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1167);
              goto end_block_exception_1706;
            }
          }
        }
      }
    }
    struct zTrieRootResult z3zE1250;
    {
      struct zTrieItemSink zupdated_sink;
      CREATE(zTrieItemSink)(&zupdated_sink);
      COPY(zTrieItemSink)(&zupdated_sink, z2zE1167.ztup0);
      struct zTrieUpdateCursor zremaining;
      CREATE(zTrieUpdateCursor)(&zremaining);
      COPY(zTrieUpdateCursor)(&zremaining, z2zE1167.ztup1);
      bool z2zE1168;
      z2zE1168 = zupdates_empty(zremaining);
      if (z2zE1168) {
        sail_fixed_bytes_32 z2zE1170;
        {
          struct zTrieItemSink z2zE1169;
          CREATE(zTrieItemSink)(&z2zE1169);
          {
            ztrie_sink_finish(&z2zE1169, zupdated_sink);
            if (have_exception) {
              KILL(zTrieItemSink)(&zsink);
              KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1167);
              KILL(zTrieItemSink)(&zupdated_sink);
              KILL(zTrieUpdateCursor)(&zremaining);
              KILL(zTrieItemSink)(&z2zE1169);
              goto end_block_exception_1706;
            }
          }
          {
            z2zE1170 = ztrie_sink_root(z2zE1169);
            if (have_exception) {
              KILL(zTrieItemSink)(&zsink);
              KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1167);
              KILL(zTrieItemSink)(&zupdated_sink);
              KILL(zTrieUpdateCursor)(&zremaining);
              KILL(zTrieItemSink)(&z2zE1169);
              goto end_block_exception_1706;
            }
          }
          KILL(zTrieItemSink)(&z2zE1169);
        }
        struct zTrieRootResult z3zE1252;
        z3zE1252.zchanged = true;
        z3zE1252.zroot = z2zE1170;
        z3zE1250 = z3zE1252;
      } else {
        struct zexception z2zE1171;
        CREATE(zexception)(&z2zE1171);
        zInvalidBlock(&z2zE1171, zWitnessDeficient);
        COPY(zexception)(current_exception, z2zE1171);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/lib/mpt/trie.sail:246.12-246.48");
        KILL(zTrieItemSink)(&zsink);
        KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1167);
        KILL(zTrieItemSink)(&zupdated_sink);
        KILL(zTrieUpdateCursor)(&zremaining);
        KILL(zexception)(&z2zE1171);
        goto end_block_exception_1706;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1171);
      }
      KILL(zTrieUpdateCursor)(&zremaining);
      KILL(zTrieItemSink)(&zupdated_sink);
      goto finish_match_1703;
    }
  case_1704: ;
    sail_match_failure("trie_root_cursor");
  finish_match_1703: ;
    z8zE553 = z3zE1250;
    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1167);
    KILL(zTrieItemSink)(&zsink);
  }
end_function_1705: ;
  return z8zE553;
end_block_exception_1706: ;
  struct zTrieRootResult z8zE1006 = { .zchanged = false, .zroot = fixed_bytes_32_zero() };
  return z8zE1006;
}

struct zTrieRootResult ztrie_root(sail_fixed_bytes_32 zbase_root, struct zTrieUpdateSource zsource)
{
  struct zTrieRootResult z8zE554;
  struct zTrieUpdateCursor z2zE1161;
  CREATE(zTrieUpdateCursor)(&z2zE1161);
  ztrie_updates_begin(&z2zE1161, zsource);
  {
    z8zE554 = ztrie_root_cursor(zbase_root, z2zE1161);
    if (have_exception) {
      KILL(zTrieUpdateCursor)(&z2zE1161);
      goto end_block_exception_1702;
    }
  }
  KILL(zTrieUpdateCursor)(&z2zE1161);
end_function_1701: ;
  return z8zE554;
end_block_exception_1702: ;
  struct zTrieRootResult z8zE1007 = { .zchanged = false, .zroot = fixed_bytes_32_zero() };
  return z8zE1007;
}

