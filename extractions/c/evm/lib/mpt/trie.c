/* Generated from sail/lib/mpt/trie.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
void zemit_live_updates_under(struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 *z8zE552, struct zTrieItemSink zsink, struct zTrieUpdateCursor zupdates, struct zTriePath zprefix)
{
  struct zoptionzIRTrieUpdatezK z2zE1239;
  CREATE(zoptionzIRTrieUpdatezK)(&z2zE1239);
  COPY(zoptionzIRTrieUpdatezK)(&z2zE1239, zupdates.zpending);
  struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z3zE1304;
  CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1304);
  {
    if (z2zE1239.kind != Kind_zNonezIRTrieUpdatezK) goto case_1752;
    COPY(zTrieItemSink)(&((&z3zE1304)->ztup0), zsink);
    COPY(zTrieUpdateCursor)(&((&z3zE1304)->ztup1), zupdates);
    goto finish_match_1747;
  }
case_1752: ;
  {
    /* complete */
    struct zTrieUpdate zupdate;
    CREATE(zTrieUpdate)(&zupdate);
    COPY(zTrieUpdate)(&zupdate, z2zE1239.variants.zSomezIRTrieUpdatezK);
    bool z2zE1241;
    {
      struct zTriePath z2zE1240;
      z2zE1240 = zupdate.zkey;
      {
        z2zE1241 = zpath_prefix_of(zprefix, z2zE1240);
        if (have_exception) {
          KILL(zoptionzIRTrieUpdatezK)(&z2zE1239);
          KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1304);
          KILL(zTrieUpdate)(&zupdate);
          goto end_block_exception_1754;
        }
      }
    }
    if (z2zE1241) {
      struct zTrieItemSink znext_sink;
      CREATE(zTrieItemSink)(&znext_sink);
      {
        struct zTrieChange z2zE1243;
        CREATE(zTrieChange)(&z2zE1243);
        COPY(zTrieChange)(&z2zE1243, zupdate.zchange);
        struct zTrieItemSink z3zE1307;
        CREATE(zTrieItemSink)(&z3zE1307);
        {
          if (z2zE1243.kind != Kind_zTrieDelete) goto case_1751;
          COPY(zTrieItemSink)(&z3zE1307, zsink);
          goto finish_match_1749;
        }
      case_1751: ;
        {
          /* complete */
          struct zByteSliceFields zvalue;
          zvalue = z2zE1243.variants.zTriePut;
          struct zTrieItem z2zE1245;
          CREATE(zTrieItem)(&z2zE1245);
          {
            struct zTriePath z2zE1244;
            z2zE1244 = zupdate.zkey;
            zitem_leaf(&z2zE1245, z2zE1244, zvalue);
          }
          {
            ztrie_sink_emit(&z3zE1307, zsink, z2zE1245);
            if (have_exception) {
              KILL(zoptionzIRTrieUpdatezK)(&z2zE1239);
              KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1304);
              KILL(zTrieUpdate)(&zupdate);
              KILL(zTrieItemSink)(&znext_sink);
              KILL(zTrieChange)(&z2zE1243);
              KILL(zTrieItemSink)(&z3zE1307);
              KILL(zTrieItem)(&z2zE1245);
              goto end_block_exception_1754;
            }
          }
          KILL(zTrieItem)(&z2zE1245);
          goto finish_match_1749;
        }
      case_1750: ;
      finish_match_1749: ;
        COPY(zTrieItemSink)(&znext_sink, z3zE1307);
        KILL(zTrieItemSink)(&z3zE1307);
        KILL(zTrieChange)(&z2zE1243);
      }
      struct zTrieUpdateCursor z2zE1242;
      CREATE(zTrieUpdateCursor)(&z2zE1242);
      ztrie_updates_advance(&z2zE1242, zupdates);
      {
        zemit_live_updates_under(&z3zE1304, znext_sink, z2zE1242, zprefix);
        if (have_exception) {
          KILL(zoptionzIRTrieUpdatezK)(&z2zE1239);
          KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1304);
          KILL(zTrieUpdate)(&zupdate);
          KILL(zTrieItemSink)(&znext_sink);
          KILL(zTrieUpdateCursor)(&z2zE1242);
          goto end_block_exception_1754;
        }
      }
      KILL(zTrieUpdateCursor)(&z2zE1242);
      KILL(zTrieItemSink)(&znext_sink);
    } else {
      struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z3zE1306;
      CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1306);
      COPY(zTrieItemSink)(&((&z3zE1306)->ztup0), zsink);
      COPY(zTrieUpdateCursor)(&((&z3zE1306)->ztup1), zupdates);
      COPY(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1304, z3zE1306);
      KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1306);
    }
    KILL(zTrieUpdate)(&zupdate);
    goto finish_match_1747;
  }
case_1748: ;
finish_match_1747: ;
  COPY(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)((*(&z8zE552)), z3zE1304);
  KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1304);
  KILL(zoptionzIRTrieUpdatezK)(&z2zE1239);
end_function_1753: ;
  goto end_function_4024;
end_block_exception_1754: ;
  goto end_function_4024;
end_function_4024: ;
}

void zemit_updates_before_child(struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 *z8zE553, struct zTrieItemSink zsink, struct zTrieUpdateCursor zupdates, struct zTriePath zprefix, struct zTriePath zchild)
{
  struct zoptionzIRTrieUpdatezK z2zE1226;
  CREATE(zoptionzIRTrieUpdatezK)(&z2zE1226);
  COPY(zoptionzIRTrieUpdatezK)(&z2zE1226, zupdates.zpending);
  struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z3zE1294;
  CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1294);
  {
    if (z2zE1226.kind != Kind_zNonezIRTrieUpdatezK) goto case_1744;
    COPY(zTrieItemSink)(&((&z3zE1294)->ztup0), zsink);
    COPY(zTrieUpdateCursor)(&((&z3zE1294)->ztup1), zupdates);
    goto finish_match_1739;
  }
case_1744: ;
  {
    /* complete */
    struct zTrieUpdate zupdate;
    CREATE(zTrieUpdate)(&zupdate);
    COPY(zTrieUpdate)(&zupdate, z2zE1226.variants.zSomezIRTrieUpdatezK);
    bool z2zE1234;
    {
      bool z2zE1233;
      {
        bool z2zE1228;
        {
          struct zTriePath z2zE1227;
          z2zE1227 = zupdate.zkey;
          {
            z2zE1228 = zpath_prefix_of(zprefix, z2zE1227);
            if (have_exception) {
              KILL(zoptionzIRTrieUpdatezK)(&z2zE1226);
              KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1294);
              KILL(zTrieUpdate)(&zupdate);
              goto end_block_exception_1746;
            }
          }
        }
        z2zE1233 = not(z2zE1228);
      }
      bool z3zE1296;
      if (z2zE1233) {  z3zE1296 = true;  } else {
        bool z2zE1232;
        {
          struct zTriePath z2zE1229;
          z2zE1229 = zupdate.zkey;
          {
            z2zE1232 = zpath_prefix_of(zchild, z2zE1229);
            if (have_exception) {
              KILL(zoptionzIRTrieUpdatezK)(&z2zE1226);
              KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1294);
              KILL(zTrieUpdate)(&zupdate);
              goto end_block_exception_1746;
            }
          }
        }
        bool z3zE1295;
        if (z2zE1232) {  z3zE1295 = true;  } else {
          bool z2zE1231;
          {
            struct zTriePath z2zE1230;
            z2zE1230 = zupdate.zkey;
            z2zE1231 = zpath_lt(z2zE1230, zchild);
          }
          z3zE1295 = not(z2zE1231);
        }
        z3zE1296 = z3zE1295;
      }
      z2zE1234 = z3zE1296;
    }
    if (z2zE1234) {
      struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z3zE1301;
      CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1301);
      COPY(zTrieItemSink)(&((&z3zE1301)->ztup0), zsink);
      COPY(zTrieUpdateCursor)(&((&z3zE1301)->ztup1), zupdates);
      COPY(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1294, z3zE1301);
      KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1301);
    } else {
      struct zTrieItemSink znext_sink;
      CREATE(zTrieItemSink)(&znext_sink);
      {
        struct zTrieChange z2zE1236;
        CREATE(zTrieChange)(&z2zE1236);
        COPY(zTrieChange)(&z2zE1236, zupdate.zchange);
        struct zTrieItemSink z3zE1298;
        CREATE(zTrieItemSink)(&z3zE1298);
        {
          if (z2zE1236.kind != Kind_zTrieDelete) goto case_1743;
          COPY(zTrieItemSink)(&z3zE1298, zsink);
          goto finish_match_1741;
        }
      case_1743: ;
        {
          /* complete */
          struct zByteSliceFields zvalue;
          zvalue = z2zE1236.variants.zTriePut;
          struct zTrieItem z2zE1238;
          CREATE(zTrieItem)(&z2zE1238);
          {
            struct zTriePath z2zE1237;
            z2zE1237 = zupdate.zkey;
            zitem_leaf(&z2zE1238, z2zE1237, zvalue);
          }
          {
            ztrie_sink_emit(&z3zE1298, zsink, z2zE1238);
            if (have_exception) {
              KILL(zoptionzIRTrieUpdatezK)(&z2zE1226);
              KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1294);
              KILL(zTrieUpdate)(&zupdate);
              KILL(zTrieItemSink)(&znext_sink);
              KILL(zTrieChange)(&z2zE1236);
              KILL(zTrieItemSink)(&z3zE1298);
              KILL(zTrieItem)(&z2zE1238);
              goto end_block_exception_1746;
            }
          }
          KILL(zTrieItem)(&z2zE1238);
          goto finish_match_1741;
        }
      case_1742: ;
      finish_match_1741: ;
        COPY(zTrieItemSink)(&znext_sink, z3zE1298);
        KILL(zTrieItemSink)(&z3zE1298);
        KILL(zTrieChange)(&z2zE1236);
      }
      struct zTrieUpdateCursor z2zE1235;
      CREATE(zTrieUpdateCursor)(&z2zE1235);
      ztrie_updates_advance(&z2zE1235, zupdates);
      {
        zemit_updates_before_child(&z3zE1294, znext_sink, z2zE1235, zprefix, zchild);
        if (have_exception) {
          KILL(zoptionzIRTrieUpdatezK)(&z2zE1226);
          KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1294);
          KILL(zTrieUpdate)(&zupdate);
          KILL(zTrieItemSink)(&znext_sink);
          KILL(zTrieUpdateCursor)(&z2zE1235);
          goto end_block_exception_1746;
        }
      }
      KILL(zTrieUpdateCursor)(&z2zE1235);
      KILL(zTrieItemSink)(&znext_sink);
    }
    KILL(zTrieUpdate)(&zupdate);
    goto finish_match_1739;
  }
case_1740: ;
finish_match_1739: ;
  COPY(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)((*(&z8zE553)), z3zE1294);
  KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1294);
  KILL(zoptionzIRTrieUpdatezK)(&z2zE1226);
end_function_1745: ;
  goto end_function_4023;
end_block_exception_1746: ;
  goto end_function_4023;
end_function_4023: ;
}

void zemit_leaf_overlay(struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 *z8zE554, struct zTrieItemSink zsink, struct zTrieUpdateCursor zupdates, struct zTriePath zprefix, struct zTriePath zkey, struct zByteSliceFields zvalue)
{
  struct zoptionzIRTrieUpdatezK z2zE1205;
  CREATE(zoptionzIRTrieUpdatezK)(&z2zE1205);
  COPY(zoptionzIRTrieUpdatezK)(&z2zE1205, zupdates.zpending);
  struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z3zE1281;
  CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1281);
  {
    if (z2zE1205.kind != Kind_zNonezIRTrieUpdatezK) goto case_1736;
    struct zTrieItemSink z2zE1207;
    CREATE(zTrieItemSink)(&z2zE1207);
    {
      struct zTrieItem z2zE1206;
      CREATE(zTrieItem)(&z2zE1206);
      zitem_leaf(&z2zE1206, zkey, zvalue);
      {
        ztrie_sink_emit(&z2zE1207, zsink, z2zE1206);
        if (have_exception) {
          KILL(zoptionzIRTrieUpdatezK)(&z2zE1205);
          KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1281);
          KILL(zTrieItemSink)(&z2zE1207);
          KILL(zTrieItem)(&z2zE1206);
          goto end_block_exception_1738;
        }
      }
      KILL(zTrieItem)(&z2zE1206);
    }
    COPY(zTrieItemSink)(&((&z3zE1281)->ztup0), z2zE1207);
    COPY(zTrieUpdateCursor)(&((&z3zE1281)->ztup1), zupdates);
    KILL(zTrieItemSink)(&z2zE1207);
    goto finish_match_1728;
  }
case_1736: ;
  {
    /* complete */
    struct zTrieUpdate zupdate;
    CREATE(zTrieUpdate)(&zupdate);
    COPY(zTrieUpdate)(&zupdate, z2zE1205.variants.zSomezIRTrieUpdatezK);
    bool z2zE1210;
    {
      bool z2zE1209;
      {
        struct zTriePath z2zE1208;
        z2zE1208 = zupdate.zkey;
        {
          z2zE1209 = zpath_prefix_of(zprefix, z2zE1208);
          if (have_exception) {
            KILL(zoptionzIRTrieUpdatezK)(&z2zE1205);
            KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1281);
            KILL(zTrieUpdate)(&zupdate);
            goto end_block_exception_1738;
          }
        }
      }
      z2zE1210 = not(z2zE1209);
    }
    if (z2zE1210) {
      struct zTrieItemSink z2zE1212;
      CREATE(zTrieItemSink)(&z2zE1212);
      {
        struct zTrieItem z2zE1211;
        CREATE(zTrieItem)(&z2zE1211);
        zitem_leaf(&z2zE1211, zkey, zvalue);
        {
          ztrie_sink_emit(&z2zE1212, zsink, z2zE1211);
          if (have_exception) {
            KILL(zoptionzIRTrieUpdatezK)(&z2zE1205);
            KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1281);
            KILL(zTrieUpdate)(&zupdate);
            KILL(zTrieItemSink)(&z2zE1212);
            KILL(zTrieItem)(&z2zE1211);
            goto end_block_exception_1738;
          }
        }
        KILL(zTrieItem)(&z2zE1211);
      }
      struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z3zE1289;
      CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1289);
      COPY(zTrieItemSink)(&((&z3zE1289)->ztup0), z2zE1212);
      COPY(zTrieUpdateCursor)(&((&z3zE1289)->ztup1), zupdates);
      COPY(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1281, z3zE1289);
      KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1289);
      KILL(zTrieItemSink)(&z2zE1212);
    } else {
      bool z2zE1214;
      {
        struct zTriePath z2zE1213;
        z2zE1213 = zupdate.zkey;
        z2zE1214 = zpath_eq(z2zE1213, zkey);
      }
      if (z2zE1214) {
        struct zTrieItemSink zupdated_sink;
        CREATE(zTrieItemSink)(&zupdated_sink);
        {
          struct zTrieChange z2zE1216;
          CREATE(zTrieChange)(&z2zE1216);
          COPY(zTrieChange)(&z2zE1216, zupdate.zchange);
          struct zTrieItemSink z3zE1286;
          CREATE(zTrieItemSink)(&z3zE1286);
          {
            if (z2zE1216.kind != Kind_zTrieDelete) goto case_1735;
            COPY(zTrieItemSink)(&z3zE1286, zsink);
            goto finish_match_1733;
          }
        case_1735: ;
          {
            /* complete */
            struct zByteSliceFields zupdated;
            zupdated = z2zE1216.variants.zTriePut;
            struct zTrieItem z2zE1217;
            CREATE(zTrieItem)(&z2zE1217);
            zitem_leaf(&z2zE1217, zkey, zupdated);
            {
              ztrie_sink_emit(&z3zE1286, zsink, z2zE1217);
              if (have_exception) {
                KILL(zoptionzIRTrieUpdatezK)(&z2zE1205);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1281);
                KILL(zTrieUpdate)(&zupdate);
                KILL(zTrieItemSink)(&zupdated_sink);
                KILL(zTrieChange)(&z2zE1216);
                KILL(zTrieItemSink)(&z3zE1286);
                KILL(zTrieItem)(&z2zE1217);
                goto end_block_exception_1738;
              }
            }
            KILL(zTrieItem)(&z2zE1217);
            goto finish_match_1733;
          }
        case_1734: ;
        finish_match_1733: ;
          COPY(zTrieItemSink)(&zupdated_sink, z3zE1286);
          KILL(zTrieItemSink)(&z3zE1286);
          KILL(zTrieChange)(&z2zE1216);
        }
        struct zTrieUpdateCursor z2zE1215;
        CREATE(zTrieUpdateCursor)(&z2zE1215);
        ztrie_updates_advance(&z2zE1215, zupdates);
        {
          zemit_live_updates_under(&z3zE1281, zupdated_sink, z2zE1215, zprefix);
          if (have_exception) {
            KILL(zoptionzIRTrieUpdatezK)(&z2zE1205);
            KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1281);
            KILL(zTrieUpdate)(&zupdate);
            KILL(zTrieItemSink)(&zupdated_sink);
            KILL(zTrieUpdateCursor)(&z2zE1215);
            goto end_block_exception_1738;
          }
        }
        KILL(zTrieUpdateCursor)(&z2zE1215);
        KILL(zTrieItemSink)(&zupdated_sink);
      } else {
        bool z2zE1219;
        {
          struct zTriePath z2zE1218;
          z2zE1218 = zupdate.zkey;
          z2zE1219 = zpath_lt(zkey, z2zE1218);
        }
        if (z2zE1219) {
          struct zTrieItemSink z2zE1221;
          CREATE(zTrieItemSink)(&z2zE1221);
          {
            struct zTrieItem z2zE1220;
            CREATE(zTrieItem)(&z2zE1220);
            zitem_leaf(&z2zE1220, zkey, zvalue);
            {
              ztrie_sink_emit(&z2zE1221, zsink, z2zE1220);
              if (have_exception) {
                KILL(zoptionzIRTrieUpdatezK)(&z2zE1205);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1281);
                KILL(zTrieUpdate)(&zupdate);
                KILL(zTrieItemSink)(&z2zE1221);
                KILL(zTrieItem)(&z2zE1220);
                goto end_block_exception_1738;
              }
            }
            KILL(zTrieItem)(&z2zE1220);
          }
          {
            zemit_live_updates_under(&z3zE1281, z2zE1221, zupdates, zprefix);
            if (have_exception) {
              KILL(zoptionzIRTrieUpdatezK)(&z2zE1205);
              KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1281);
              KILL(zTrieUpdate)(&zupdate);
              KILL(zTrieItemSink)(&z2zE1221);
              goto end_block_exception_1738;
            }
          }
          KILL(zTrieItemSink)(&z2zE1221);
        } else {
          struct zTrieItemSink z3zE1292;
          CREATE(zTrieItemSink)(&z3zE1292);
          {
            struct zTrieChange z2zE1223;
            CREATE(zTrieChange)(&z2zE1223);
            COPY(zTrieChange)(&z2zE1223, zupdate.zchange);
            struct zTrieItemSink z3zE1283;
            CREATE(zTrieItemSink)(&z3zE1283);
            {
              if (z2zE1223.kind != Kind_zTrieDelete) goto case_1732;
              COPY(zTrieItemSink)(&z3zE1283, zsink);
              goto finish_match_1730;
            }
          case_1732: ;
            {
              /* complete */
              struct zByteSliceFields z3zE1293;
              z3zE1293 = z2zE1223.variants.zTriePut;
              struct zTrieItem z2zE1225;
              CREATE(zTrieItem)(&z2zE1225);
              {
                struct zTriePath z2zE1224;
                z2zE1224 = zupdate.zkey;
                zitem_leaf(&z2zE1225, z2zE1224, z3zE1293);
              }
              {
                ztrie_sink_emit(&z3zE1283, zsink, z2zE1225);
                if (have_exception) {
                  KILL(zoptionzIRTrieUpdatezK)(&z2zE1205);
                  KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1281);
                  KILL(zTrieUpdate)(&zupdate);
                  KILL(zTrieItemSink)(&z3zE1292);
                  KILL(zTrieChange)(&z2zE1223);
                  KILL(zTrieItemSink)(&z3zE1283);
                  KILL(zTrieItem)(&z2zE1225);
                  goto end_block_exception_1738;
                }
              }
              KILL(zTrieItem)(&z2zE1225);
              goto finish_match_1730;
            }
          case_1731: ;
          finish_match_1730: ;
            COPY(zTrieItemSink)(&z3zE1292, z3zE1283);
            KILL(zTrieItemSink)(&z3zE1283);
            KILL(zTrieChange)(&z2zE1223);
          }
          struct zTrieUpdateCursor z2zE1222;
          CREATE(zTrieUpdateCursor)(&z2zE1222);
          ztrie_updates_advance(&z2zE1222, zupdates);
          {
            zemit_leaf_overlay(&z3zE1281, z3zE1292, z2zE1222, zprefix, zkey, zvalue);
            if (have_exception) {
              KILL(zoptionzIRTrieUpdatezK)(&z2zE1205);
              KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1281);
              KILL(zTrieUpdate)(&zupdate);
              KILL(zTrieItemSink)(&z3zE1292);
              KILL(zTrieUpdateCursor)(&z2zE1222);
              goto end_block_exception_1738;
            }
          }
          KILL(zTrieUpdateCursor)(&z2zE1222);
          KILL(zTrieItemSink)(&z3zE1292);
        }
      }
    }
    KILL(zTrieUpdate)(&zupdate);
    goto finish_match_1728;
  }
case_1729: ;
finish_match_1728: ;
  COPY(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)((*(&z8zE554)), z3zE1281);
  KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1281);
  KILL(zoptionzIRTrieUpdatezK)(&z2zE1205);
end_function_1737: ;
  goto end_function_4022;
end_block_exception_1738: ;
  goto end_function_4022;
end_function_4022: ;
}

void zwitness_emit(struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 *z8zE555, struct zByteSliceFields znode, struct zTriePath zprefix, struct zTrieUpdateCursor zupdates, struct zTrieItemSink zsink, uint64_t zcursor)
{
  bool z2zE1173;
  {
    uint64_t z2zE1172;
    z2zE1172 = znode.zlen;
    z2zE1173 = (z2zE1172 == UINT64_C(0));
  }
  if (z2zE1173) {
    {
      zemit_live_updates_under((*(&z8zE555)), zsink, zupdates, zprefix);
      if (have_exception) {  goto end_block_exception_1727;  }
    }
  } else {
    struct zTrieNode z2zE1174;
    CREATE(zTrieNode)(&z2zE1174);
    {
      zdecode_trie_node(&z2zE1174, znode);
      if (have_exception) {
        KILL(zTrieNode)(&z2zE1174);
        goto end_block_exception_1727;
      }
    }
    struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z3zE1254;
    CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
    {
      if (z2zE1174.kind != Kind_zLeafNode) goto case_1725;
      struct zLeafNodeData zleaf;
      zleaf = z2zE1174.variants.zLeafNode;
      struct zTriePath zkey;
      {
        struct zTriePath z2zE1177;
        z2zE1177 = zleaf.zpath;
        {
          zkey = zpath_concat(zprefix, z2zE1177);
          if (have_exception) {
            KILL(zTrieNode)(&z2zE1174);
            KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
            goto end_block_exception_1727;
          }
        }
      }
      struct zByteSliceFields z2zE1176;
      {
        struct zRlpFieldRefFields z2zE1175;
        z2zE1175 = zleaf.zvalue;
        z2zE1176 = zrlp_ref_content(z2zE1175);
      }
      {
        zemit_leaf_overlay(&z3zE1254, zsink, zupdates, zprefix, zkey, z2zE1176);
        if (have_exception) {
          KILL(zTrieNode)(&z2zE1174);
          KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
          goto end_block_exception_1727;
        }
      }
      goto finish_match_1710;
    }
  case_1725: ;
    {
      if (z2zE1174.kind != Kind_zExtensionNode) goto case_1720;
      struct zExtensionNodeData zextension;
      zextension = z2zE1174.variants.zExtensionNode;
      uint64_t zextension_len;
      {
        struct zTriePath z2zE1193;
        z2zE1193 = zextension.zpath;
        zextension_len = zpath_len(z2zE1193);
      }
      uint64_t znext_cursor;
      {    znext_cursor = (zcursor + zextension_len);
      }
      bool z2zE1179;
      {
        bool z2zE1178;
        z2zE1178 = (zextension_len == UINT64_C(0));
        bool z3zE1271;
        if (z2zE1178) {  z3zE1271 = true;  } else {  z3zE1271 = (UINT64_C(64) < znext_cursor);  }
        z2zE1179 = z3zE1271;
      }
      if (z2zE1179) {
        struct zexception z2zE1180;
        CREATE(zexception)(&z2zE1180);
        zInvalidBlock(&z2zE1180, zWitnessDeficient);
        COPY(zexception)(current_exception, z2zE1180);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/lib/mpt/trie.sail:143.20-143.56");
        KILL(zTrieNode)(&z2zE1174);
        KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
        KILL(zexception)(&z2zE1180);
        goto end_block_exception_1727;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1180);
      } else {
        struct zTriePath zchild_prefix;
        {
          struct zTriePath z2zE1192;
          z2zE1192 = zextension.zpath;
          {
            zchild_prefix = zpath_concat(zprefix, z2zE1192);
            if (have_exception) {
              KILL(zTrieNode)(&z2zE1174);
              KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
              goto end_block_exception_1727;
            }
          }
        }
        struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z2zE1181;
        CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1181);
        {
          zemit_updates_before_child(&z2zE1181, zsink, zupdates, zprefix, zchild_prefix);
          if (have_exception) {
            KILL(zTrieNode)(&z2zE1174);
            KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
            KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1181);
            goto end_block_exception_1727;
          }
        }
        struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z3zE1273;
        CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1273);
        {
          struct zTrieItemSink zbefore_sink;
          CREATE(zTrieItemSink)(&zbefore_sink);
          COPY(zTrieItemSink)(&zbefore_sink, z2zE1181.ztup0);
          struct zTrieUpdateCursor zchild_updates;
          CREATE(zTrieUpdateCursor)(&zchild_updates);
          COPY(zTrieUpdateCursor)(&zchild_updates, z2zE1181.ztup1);
          struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z2zE1191;
          CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1191);
          {
            bool z2zE1182;
            {
              z2zE1182 = znext_update_under(zchild_updates, zchild_prefix);
              if (have_exception) {
                KILL(zTrieNode)(&z2zE1174);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1181);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1273);
                KILL(zTrieItemSink)(&zbefore_sink);
                KILL(zTrieUpdateCursor)(&zchild_updates);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1191);
                goto end_block_exception_1727;
              }
            }
            if (z2zE1182) {
              struct zByteSliceFields zchild;
              {
                struct zNodeRef z2zE1186;
                CREATE(zNodeRef)(&z2zE1186);
                {
                  struct zRlpFieldRefFields z2zE1185;
                  z2zE1185 = zextension.zchild;
                  {
                    zfield_to_ref(&z2zE1186, z2zE1185);
                    if (have_exception) {
                      KILL(zTrieNode)(&z2zE1174);
                      KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
                      KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1181);
                      KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1273);
                      KILL(zTrieItemSink)(&zbefore_sink);
                      KILL(zTrieUpdateCursor)(&zchild_updates);
                      KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1191);
                      KILL(zNodeRef)(&z2zE1186);
                      goto end_block_exception_1727;
                    }
                  }
                }
                {
                  zchild = zresolve_ref(z2zE1186);
                  if (have_exception) {
                    KILL(zTrieNode)(&z2zE1174);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1181);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1273);
                    KILL(zTrieItemSink)(&zbefore_sink);
                    KILL(zTrieUpdateCursor)(&zchild_updates);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1191);
                    KILL(zNodeRef)(&z2zE1186);
                    goto end_block_exception_1727;
                  }
                }
                KILL(zNodeRef)(&z2zE1186);
              }
              bool z2zE1184;
              {
                uint64_t z2zE1183;
                z2zE1183 = zchild.zlen;
                z2zE1184 = (z2zE1183 == UINT64_C(0));
              }
              if (z2zE1184) {
                {
                  zemit_live_updates_under(&z2zE1191, zbefore_sink, zchild_updates, zchild_prefix);
                  if (have_exception) {
                    KILL(zTrieNode)(&z2zE1174);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1181);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1273);
                    KILL(zTrieItemSink)(&zbefore_sink);
                    KILL(zTrieUpdateCursor)(&zchild_updates);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1191);
                    goto end_block_exception_1727;
                  }
                }
              } else {
                {
                  zwitness_emit(&z2zE1191, zchild, zchild_prefix, zchild_updates, zbefore_sink, znext_cursor);
                  if (have_exception) {
                    KILL(zTrieNode)(&z2zE1174);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1181);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1273);
                    KILL(zTrieItemSink)(&zbefore_sink);
                    KILL(zTrieUpdateCursor)(&zchild_updates);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1191);
                    goto end_block_exception_1727;
                  }
                }
              }
            } else {
              struct zTrieItemSink z2zE1190;
              CREATE(zTrieItemSink)(&z2zE1190);
              {
                struct zTrieItem z2zE1189;
                CREATE(zTrieItem)(&z2zE1189);
                {
                  struct zNodeRef z2zE1188;
                  CREATE(zNodeRef)(&z2zE1188);
                  {
                    struct zRlpFieldRefFields z2zE1187;
                    z2zE1187 = zextension.zchild;
                    {
                      zfield_to_ref(&z2zE1188, z2zE1187);
                      if (have_exception) {
                        KILL(zTrieNode)(&z2zE1174);
                        KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
                        KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1181);
                        KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1273);
                        KILL(zTrieItemSink)(&zbefore_sink);
                        KILL(zTrieUpdateCursor)(&zchild_updates);
                        KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1191);
                        KILL(zTrieItemSink)(&z2zE1190);
                        KILL(zTrieItem)(&z2zE1189);
                        KILL(zNodeRef)(&z2zE1188);
                        goto end_block_exception_1727;
                      }
                    }
                  }
                  zitem_branch(&z2zE1189, zchild_prefix, z2zE1188);
                  KILL(zNodeRef)(&z2zE1188);
                }
                {
                  ztrie_sink_emit(&z2zE1190, zbefore_sink, z2zE1189);
                  if (have_exception) {
                    KILL(zTrieNode)(&z2zE1174);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1181);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1273);
                    KILL(zTrieItemSink)(&zbefore_sink);
                    KILL(zTrieUpdateCursor)(&zchild_updates);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1191);
                    KILL(zTrieItemSink)(&z2zE1190);
                    KILL(zTrieItem)(&z2zE1189);
                    goto end_block_exception_1727;
                  }
                }
                KILL(zTrieItem)(&z2zE1189);
              }
              struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z3zE1274;
              CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1274);
              COPY(zTrieItemSink)(&((&z3zE1274)->ztup0), z2zE1190);
              COPY(zTrieUpdateCursor)(&((&z3zE1274)->ztup1), zchild_updates);
              COPY(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1191, z3zE1274);
              KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1274);
              KILL(zTrieItemSink)(&z2zE1190);
            }
          }
          struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z3zE1275;
          CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1275);
          {
            struct zTrieItemSink zchild_sink;
            CREATE(zTrieItemSink)(&zchild_sink);
            COPY(zTrieItemSink)(&zchild_sink, z2zE1191.ztup0);
            struct zTrieUpdateCursor zlater_updates;
            CREATE(zTrieUpdateCursor)(&zlater_updates);
            COPY(zTrieUpdateCursor)(&zlater_updates, z2zE1191.ztup1);
            {
              zemit_live_updates_under(&z3zE1275, zchild_sink, zlater_updates, zprefix);
              if (have_exception) {
                KILL(zTrieNode)(&z2zE1174);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1181);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1273);
                KILL(zTrieItemSink)(&zbefore_sink);
                KILL(zTrieUpdateCursor)(&zchild_updates);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1191);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1275);
                KILL(zTrieItemSink)(&zchild_sink);
                KILL(zTrieUpdateCursor)(&zlater_updates);
                goto end_block_exception_1727;
              }
            }
            KILL(zTrieUpdateCursor)(&zlater_updates);
            KILL(zTrieItemSink)(&zchild_sink);
            goto finish_match_1723;
          }
        case_1724: ;
          sail_match_failure("witness_emit");
        finish_match_1723: ;
          COPY(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1273, z3zE1275);
          KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1275);
          KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1191);
          KILL(zTrieUpdateCursor)(&zchild_updates);
          KILL(zTrieItemSink)(&zbefore_sink);
          goto finish_match_1721;
        }
      case_1722: ;
        sail_match_failure("witness_emit");
      finish_match_1721: ;
        COPY(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254, z3zE1273);
        KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1273);
        KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1181);
      }
      goto finish_match_1710;
    }
  case_1720: ;
    {
      if (z2zE1174.kind != Kind_zBranchNode) goto case_1712;
      struct zBranchNodeData zbranch;
      CREATE(zBranchNodeData)(&zbranch);
      COPY(zBranchNodeData)(&zbranch, z2zE1174.variants.zBranchNode);
      struct zRlpFieldRefFields zbranch_value;
      zbranch_value = zbranch.zvalue;
      bool z2zE1196;
      {
        bool z2zE1195;
        {
          uint64_t z2zE1194;
          z2zE1194 = zbranch_value.zcontent_len;
          z2zE1195 = (z2zE1194 != UINT64_C(0));
        }
        bool z3zE1256;
        if (z2zE1195) {  z3zE1256 = true;  } else {  z3zE1256 = (!(zcursor < UINT64_C(64)));  }
        z2zE1196 = z3zE1256;
      }
      if (z2zE1196) {
        struct zexception z2zE1197;
        CREATE(zexception)(&z2zE1197);
        zInvalidBlock(&z2zE1197, zWitnessDeficient);
        COPY(zexception)(current_exception, z2zE1197);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/lib/mpt/trie.sail:166.20-166.56");
        KILL(zTrieNode)(&z2zE1174);
        KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
        KILL(zBranchNodeData)(&zbranch);
        KILL(zexception)(&z2zE1197);
        goto end_block_exception_1727;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1197);
      } else {
        uint64_t z3zE1279;
        {    z3zE1279 = (zcursor + UINT64_C(1));
        }
        struct zTrieItemSink zcurrent_sink;
        CREATE(zTrieItemSink)(&zcurrent_sink);
        COPY(zTrieItemSink)(&zcurrent_sink, zsink);
        struct zTrieUpdateCursor zremaining;
        CREATE(zTrieUpdateCursor)(&zremaining);
        COPY(zTrieUpdateCursor)(&zremaining, zupdates);
        uint64_t znib;
        znib = UINT64_C(0x0);
        int64_t z3zE1258;
        {    z3zE1258 = (int64_t)(UINT64_C(0));
        }
        int64_t z3zE1259;
        {    z3zE1259 = (int64_t)(UINT64_C(15));
        }
        int64_t z3zE1260;
        {    z3zE1260 = (int64_t)(UINT64_C(1));
        }
        {
          int64_t zi;
          zi = z3zE1258;
          unit z3zE1268;
        for_start_1713: ;
          {
            if ((z3zE1259 < zi)) goto for_end_1714;
            struct zRlpFieldRefFields zfield;
            {
              zz5vecz8z5structz0zzRlpFieldRefFieldsz9 z2zE1203;
              CREATE(zz5vecz8z5structz0zzRlpFieldRefFieldsz9)(&z2zE1203);
              COPY(zz5vecz8z5structz0zzRlpFieldRefFieldsz9)(&z2zE1203, zbranch.zchildren);
              zfield = fast_vector_access_zz5vecz8z5structz0zzRlpFieldRefFieldsz9(z2zE1203, zi);
              KILL(zz5vecz8z5structz0zzRlpFieldRefFieldsz9)(&z2zE1203);
            }
            struct zTriePath z3zE1280;
            {
              struct zTriePath z2zE1202;
              {
                z2zE1202 = zpath_single(znib);
                if (have_exception) {
                  KILL(zTrieNode)(&z2zE1174);
                  KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
                  KILL(zBranchNodeData)(&zbranch);
                  KILL(zTrieItemSink)(&zcurrent_sink);
                  KILL(zTrieUpdateCursor)(&zremaining);
                  goto end_block_exception_1727;
                }
              }
              {
                z3zE1280 = zpath_concat(zprefix, z2zE1202);
                if (have_exception) {
                  KILL(zTrieNode)(&z2zE1174);
                  KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
                  KILL(zBranchNodeData)(&zbranch);
                  KILL(zTrieItemSink)(&zcurrent_sink);
                  KILL(zTrieUpdateCursor)(&zremaining);
                  goto end_block_exception_1727;
                }
              }
            }
            struct zNodeRef zchildref;
            CREATE(zNodeRef)(&zchildref);
            {
              zfield_to_ref(&zchildref, zfield);
              if (have_exception) {
                KILL(zTrieNode)(&z2zE1174);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
                KILL(zBranchNodeData)(&zbranch);
                KILL(zTrieItemSink)(&zcurrent_sink);
                KILL(zTrieUpdateCursor)(&zremaining);
                KILL(zNodeRef)(&zchildref);
                goto end_block_exception_1727;
              }
            }
            bool zpresent;
            {
              bool z3zE1261;
              {
                if (zchildref.kind != Kind_zEmptyRef) goto case_1717;
                z3zE1261 = false;
                goto finish_match_1715;
              }
            case_1717: ;
              {
                z3zE1261 = true;
                goto finish_match_1715;
              }
            case_1716: ;
            finish_match_1715: ;
              zpresent = z3zE1261;
            }
            bool z2zE1198;
            {
              z2zE1198 = znext_update_under(zremaining, z3zE1280);
              if (have_exception) {
                KILL(zTrieNode)(&z2zE1174);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
                KILL(zBranchNodeData)(&zbranch);
                KILL(zTrieItemSink)(&zcurrent_sink);
                KILL(zTrieUpdateCursor)(&zremaining);
                KILL(zNodeRef)(&zchildref);
                goto end_block_exception_1727;
              }
            }
            unit z3zE1264;
            if (z2zE1198) {
              struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z2zE1200;
              CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1200);
              if (zpresent) {
                struct zByteSliceFields z2zE1199;
                {
                  z2zE1199 = zresolve_ref(zchildref);
                  if (have_exception) {
                    KILL(zTrieNode)(&z2zE1174);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
                    KILL(zBranchNodeData)(&zbranch);
                    KILL(zTrieItemSink)(&zcurrent_sink);
                    KILL(zTrieUpdateCursor)(&zremaining);
                    KILL(zNodeRef)(&zchildref);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1200);
                    goto end_block_exception_1727;
                  }
                }
                {
                  zwitness_emit(&z2zE1200, z2zE1199, z3zE1280, zremaining, zcurrent_sink, z3zE1279);
                  if (have_exception) {
                    KILL(zTrieNode)(&z2zE1174);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
                    KILL(zBranchNodeData)(&zbranch);
                    KILL(zTrieItemSink)(&zcurrent_sink);
                    KILL(zTrieUpdateCursor)(&zremaining);
                    KILL(zNodeRef)(&zchildref);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1200);
                    goto end_block_exception_1727;
                  }
                }
              } else {
                {
                  zemit_live_updates_under(&z2zE1200, zcurrent_sink, zremaining, z3zE1280);
                  if (have_exception) {
                    KILL(zTrieNode)(&z2zE1174);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
                    KILL(zBranchNodeData)(&zbranch);
                    KILL(zTrieItemSink)(&zcurrent_sink);
                    KILL(zTrieUpdateCursor)(&zremaining);
                    KILL(zNodeRef)(&zchildref);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1200);
                    goto end_block_exception_1727;
                  }
                }
              }
              unit z3zE1265;
              {
                struct zTrieItemSink znext_sink;
                CREATE(zTrieItemSink)(&znext_sink);
                COPY(zTrieItemSink)(&znext_sink, z2zE1200.ztup0);
                struct zTrieUpdateCursor znext_updates;
                CREATE(zTrieUpdateCursor)(&znext_updates);
                COPY(zTrieUpdateCursor)(&znext_updates, z2zE1200.ztup1);
                COPY(zTrieItemSink)(&zcurrent_sink, znext_sink);
                unit z3zE1266;
                z3zE1266 = UNIT;
                COPY(zTrieUpdateCursor)(&zremaining, znext_updates);
                z3zE1265 = UNIT;
                KILL(zTrieUpdateCursor)(&znext_updates);
                KILL(zTrieItemSink)(&znext_sink);
                goto finish_match_1718;
              }
            case_1719: ;
              sail_match_failure("witness_emit");
            finish_match_1718: ;
              z3zE1264 = z3zE1265;
              KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1200);
            } else if (zpresent) {
              struct zTrieItem z2zE1201;
              CREATE(zTrieItem)(&z2zE1201);
              zitem_subtree(&z2zE1201, z3zE1280, zchildref);
              {
                ztrie_sink_emit(&zcurrent_sink, zcurrent_sink, z2zE1201);
                if (have_exception) {
                  KILL(zTrieNode)(&z2zE1174);
                  KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
                  KILL(zBranchNodeData)(&zbranch);
                  KILL(zTrieItemSink)(&zcurrent_sink);
                  KILL(zTrieUpdateCursor)(&zremaining);
                  KILL(zNodeRef)(&zchildref);
                  KILL(zTrieItem)(&z2zE1201);
                  goto end_block_exception_1727;
                }
              }
              z3zE1264 = UNIT;
              KILL(zTrieItem)(&z2zE1201);
            } else {  z3zE1264 = UNIT;  }
            znib = ((znib + UINT64_C(0x1)) & UINT64_C(0xF));
            z3zE1268 = UNIT;
            KILL(zNodeRef)(&zchildref);
            zi = (zi + z3zE1260);
            goto for_start_1713;
          }
        for_end_1714: ;
        }
        unit z3zE1269;
        z3zE1269 = UNIT;
        struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z3zE1270;
        CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1270);
        COPY(zTrieItemSink)(&((&z3zE1270)->ztup0), zcurrent_sink);
        COPY(zTrieUpdateCursor)(&((&z3zE1270)->ztup1), zremaining);
        COPY(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254, z3zE1270);
        KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1270);
        KILL(zTrieUpdateCursor)(&zremaining);
        KILL(zTrieItemSink)(&zcurrent_sink);
      }
      KILL(zBranchNodeData)(&zbranch);
      goto finish_match_1710;
    }
  case_1712: ;
    {
      /* complete */
      struct zexception z2zE1204;
      CREATE(zexception)(&z2zE1204);
      zInvalidBlock(&z2zE1204, zWitnessDeficient);
      COPY(zexception)(current_exception, z2zE1204);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/lib/mpt/trie.sail:204.29-204.65");
      KILL(zTrieNode)(&z2zE1174);
      KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
      KILL(zexception)(&z2zE1204);
      goto end_block_exception_1727;
      /* unreachable after throw */
      KILL(zexception)(&z2zE1204);
      goto finish_match_1710;
    }
  case_1711: ;
  finish_match_1710: ;
    COPY(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)((*(&z8zE555)), z3zE1254);
    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE1254);
    KILL(zTrieNode)(&z2zE1174);
  }
end_function_1726: ;
  goto end_function_4021;
end_block_exception_1727: ;
  goto end_function_4021;
end_function_4021: ;
}

struct zTrieRootResult ztrie_root_cursor(sail_fixed_bytes_32 zbase_root, struct zTrieUpdateCursor zupdates)
{
  struct zTrieRootResult z8zE556;
  bool z2zE1162;
  z2zE1162 = zupdates_empty(zupdates);
  if (z2zE1162) {
    struct zTrieRootResult z3zE1253;
    z3zE1253.zchanged = false;
    z3zE1253.zroot = zbase_root;
    z8zE556 = z3zE1253;
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
        for (size_t z8zE1017 = 0; z8zE1017 < 32; ++z8zE1017) {
          z3zE1248.zdata.bytes[z8zE1017] = (uint8_t)(z3zE1249.data[z8zE1017] & UINT64_C(0xff));
        }
        KILL(zz5vecz8z5bv8z9)(&z3zE1249);
        z3zE1248.zlen = UINT64_C(0);
        {
          zemit_live_updates_under(&z2zE1167, zsink, zupdates, z3zE1248);
          if (have_exception) {
            KILL(zTrieItemSink)(&zsink);
            KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1167);
            goto end_block_exception_1709;
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
          COPY(sail_string)(throw_location, "sail/lib/mpt/trie.sail:241.20-241.56");
          KILL(zTrieItemSink)(&zsink);
          KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1167);
          KILL(zexception)(&z2zE1166);
          goto end_block_exception_1709;
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
          for (size_t z8zE1016 = 0; z8zE1016 < 32; ++z8zE1016) {
            z3zE1246.zdata.bytes[z8zE1016] = (uint8_t)(z3zE1247.data[z8zE1016] & UINT64_C(0xff));
          }
          KILL(zz5vecz8z5bv8z9)(&z3zE1247);
          z3zE1246.zlen = UINT64_C(0);
          {
            zwitness_emit(&z2zE1167, znode, z3zE1246, zupdates, zsink, UINT64_C(0));
            if (have_exception) {
              KILL(zTrieItemSink)(&zsink);
              KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1167);
              goto end_block_exception_1709;
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
              goto end_block_exception_1709;
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
              goto end_block_exception_1709;
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
        COPY(sail_string)(throw_location, "sail/lib/mpt/trie.sail:249.12-249.48");
        KILL(zTrieItemSink)(&zsink);
        KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1167);
        KILL(zTrieItemSink)(&zupdated_sink);
        KILL(zTrieUpdateCursor)(&zremaining);
        KILL(zexception)(&z2zE1171);
        goto end_block_exception_1709;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1171);
      }
      KILL(zTrieUpdateCursor)(&zremaining);
      KILL(zTrieItemSink)(&zupdated_sink);
      goto finish_match_1706;
    }
  case_1707: ;
    sail_match_failure("trie_root_cursor");
  finish_match_1706: ;
    z8zE556 = z3zE1250;
    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE1167);
    KILL(zTrieItemSink)(&zsink);
  }
end_function_1708: ;
  return z8zE556;
end_block_exception_1709: ;
  struct zTrieRootResult z8zE1018 = { .zchanged = false, .zroot = fixed_bytes_32_zero() };
  return z8zE1018;
}

struct zTrieRootResult ztrie_root(sail_fixed_bytes_32 zbase_root, struct zTrieUpdateSource zsource)
{
  struct zTrieRootResult z8zE557;
  struct zTrieUpdateCursor z2zE1161;
  CREATE(zTrieUpdateCursor)(&z2zE1161);
  ztrie_updates_begin(&z2zE1161, zsource);
  {
    z8zE557 = ztrie_root_cursor(zbase_root, z2zE1161);
    if (have_exception) {
      KILL(zTrieUpdateCursor)(&z2zE1161);
      goto end_block_exception_1705;
    }
  }
  KILL(zTrieUpdateCursor)(&z2zE1161);
end_function_1704: ;
  return z8zE557;
end_block_exception_1705: ;
  struct zTrieRootResult z8zE1019 = { .zchanged = false, .zroot = fixed_bytes_32_zero() };
  return z8zE1019;
}

