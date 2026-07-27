/* Generated from sail/lib/mpt/trie.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
void zemit_live_updates_under(struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 *z8zE598, struct zTrieItemSink zsink, struct zTrieUpdateCursor zupdates, struct zTriePath zprefix)
{
  struct zoptionzIRTrieUpdatezK z2zE789;
  CREATE(zoptionzIRTrieUpdatezK)(&z2zE789);
  z2zE789 = zupdates.zpending;
  struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z3zE984;
  CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE984);
  {
    if (z2zE789.kind != Kind_zNonezIRTrieUpdatezK) goto case_1505;
    COPY(zTrieItemSink)(&((&z3zE984)->ztup0), zsink);
    z3zE984.ztup1 = zupdates;
    goto finish_match_1500;
  }
case_1505: ;
  {
    /* complete */
    struct zTrieUpdate zupdate;
    CREATE(zTrieUpdate)(&zupdate);
    zupdate = z2zE789.variants.zSomezIRTrieUpdatezK;
    bool z2zE791;
    {
      struct zTriePath z2zE790;
      z2zE790 = zupdate.zkey;
      {
        z2zE791 = zpath_prefix_of(zprefix, z2zE790);
        if (have_exception) {
          KILL(zoptionzIRTrieUpdatezK)(&z2zE789);
          KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE984);
          KILL(zTrieUpdate)(&zupdate);
          goto end_block_exception_1507;
        }
      }
    }
    if (z2zE791) {
      struct zTrieItemSink znext_sink;
      CREATE(zTrieItemSink)(&znext_sink);
      {
        struct zTrieChange z2zE793;
        CREATE(zTrieChange)(&z2zE793);
        z2zE793 = zupdate.zchange;
        struct zTrieItemSink z3zE987;
        CREATE(zTrieItemSink)(&z3zE987);
        {
          if (z2zE793.kind != Kind_zTrieDelete) goto case_1504;
          COPY(zTrieItemSink)(&z3zE987, zsink);
          goto finish_match_1502;
        }
      case_1504: ;
        {
          /* complete */
          struct zByteSliceFields zvalue;
          zvalue = z2zE793.variants.zTriePut;
          struct zTrieItem z2zE795;
          CREATE(zTrieItem)(&z2zE795);
          {
            struct zTriePath z2zE794;
            z2zE794 = zupdate.zkey;
            zitem_leaf(&z2zE795, z2zE794, zvalue);
          }
          {
            ztrie_sink_emit(&z3zE987, zsink, z2zE795);
            if (have_exception) {
              KILL(zoptionzIRTrieUpdatezK)(&z2zE789);
              KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE984);
              KILL(zTrieUpdate)(&zupdate);
              KILL(zTrieItemSink)(&znext_sink);
              KILL(zTrieChange)(&z2zE793);
              KILL(zTrieItemSink)(&z3zE987);
              KILL(zTrieItem)(&z2zE795);
              goto end_block_exception_1507;
            }
          }
          KILL(zTrieItem)(&z2zE795);
          goto finish_match_1502;
        }
      case_1503: ;
      finish_match_1502: ;
        COPY(zTrieItemSink)(&znext_sink, z3zE987);
        KILL(zTrieItemSink)(&z3zE987);
        KILL(zTrieChange)(&z2zE793);
      }
      struct zTrieUpdateCursor z2zE792;
      CREATE(zTrieUpdateCursor)(&z2zE792);
      ztrie_updates_advance(&z2zE792, zupdates);
      {
        zemit_live_updates_under(&z3zE984, znext_sink, z2zE792, zprefix);
        if (have_exception) {
          KILL(zoptionzIRTrieUpdatezK)(&z2zE789);
          KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE984);
          KILL(zTrieUpdate)(&zupdate);
          KILL(zTrieItemSink)(&znext_sink);
          KILL(zTrieUpdateCursor)(&z2zE792);
          goto end_block_exception_1507;
        }
      }
      KILL(zTrieUpdateCursor)(&z2zE792);
      KILL(zTrieItemSink)(&znext_sink);
    } else {
      struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z3zE986;
      CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE986);
      COPY(zTrieItemSink)(&((&z3zE986)->ztup0), zsink);
      z3zE986.ztup1 = zupdates;
      COPY(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE984, z3zE986);
      KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE986);
    }
    KILL(zTrieUpdate)(&zupdate);
    goto finish_match_1500;
  }
case_1501: ;
finish_match_1500: ;
  COPY(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)((*(&z8zE598)), z3zE984);
  KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE984);
  KILL(zoptionzIRTrieUpdatezK)(&z2zE789);
end_function_1506: ;
  goto end_function_3592;
end_block_exception_1507: ;
  goto end_function_3592;
end_function_3592: ;
}

void zemit_updates_before_child(struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 *z8zE599, struct zTrieItemSink zsink, struct zTrieUpdateCursor zupdates, struct zTriePath zprefix, struct zTriePath zchild)
{
  struct zoptionzIRTrieUpdatezK z2zE776;
  CREATE(zoptionzIRTrieUpdatezK)(&z2zE776);
  z2zE776 = zupdates.zpending;
  struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z3zE974;
  CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE974);
  {
    if (z2zE776.kind != Kind_zNonezIRTrieUpdatezK) goto case_1497;
    COPY(zTrieItemSink)(&((&z3zE974)->ztup0), zsink);
    z3zE974.ztup1 = zupdates;
    goto finish_match_1492;
  }
case_1497: ;
  {
    /* complete */
    struct zTrieUpdate zupdate;
    CREATE(zTrieUpdate)(&zupdate);
    zupdate = z2zE776.variants.zSomezIRTrieUpdatezK;
    bool z2zE784;
    {
      bool z2zE783;
      {
        bool z2zE778;
        {
          struct zTriePath z2zE777;
          z2zE777 = zupdate.zkey;
          {
            z2zE778 = zpath_prefix_of(zprefix, z2zE777);
            if (have_exception) {
              KILL(zoptionzIRTrieUpdatezK)(&z2zE776);
              KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE974);
              KILL(zTrieUpdate)(&zupdate);
              goto end_block_exception_1499;
            }
          }
        }
        z2zE783 = not(z2zE778);
      }
      bool z3zE976;
      if (z2zE783) {  z3zE976 = true;  } else {
        bool z2zE782;
        {
          struct zTriePath z2zE779;
          z2zE779 = zupdate.zkey;
          {
            z2zE782 = zpath_prefix_of(zchild, z2zE779);
            if (have_exception) {
              KILL(zoptionzIRTrieUpdatezK)(&z2zE776);
              KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE974);
              KILL(zTrieUpdate)(&zupdate);
              goto end_block_exception_1499;
            }
          }
        }
        bool z3zE975;
        if (z2zE782) {  z3zE975 = true;  } else {
          bool z2zE781;
          {
            struct zTriePath z2zE780;
            z2zE780 = zupdate.zkey;
            z2zE781 = zpath_lt(z2zE780, zchild);
          }
          z3zE975 = not(z2zE781);
        }
        z3zE976 = z3zE975;
      }
      z2zE784 = z3zE976;
    }
    if (z2zE784) {
      struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z3zE981;
      CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE981);
      COPY(zTrieItemSink)(&((&z3zE981)->ztup0), zsink);
      z3zE981.ztup1 = zupdates;
      COPY(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE974, z3zE981);
      KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE981);
    } else {
      struct zTrieItemSink znext_sink;
      CREATE(zTrieItemSink)(&znext_sink);
      {
        struct zTrieChange z2zE786;
        CREATE(zTrieChange)(&z2zE786);
        z2zE786 = zupdate.zchange;
        struct zTrieItemSink z3zE978;
        CREATE(zTrieItemSink)(&z3zE978);
        {
          if (z2zE786.kind != Kind_zTrieDelete) goto case_1496;
          COPY(zTrieItemSink)(&z3zE978, zsink);
          goto finish_match_1494;
        }
      case_1496: ;
        {
          /* complete */
          struct zByteSliceFields zvalue;
          zvalue = z2zE786.variants.zTriePut;
          struct zTrieItem z2zE788;
          CREATE(zTrieItem)(&z2zE788);
          {
            struct zTriePath z2zE787;
            z2zE787 = zupdate.zkey;
            zitem_leaf(&z2zE788, z2zE787, zvalue);
          }
          {
            ztrie_sink_emit(&z3zE978, zsink, z2zE788);
            if (have_exception) {
              KILL(zoptionzIRTrieUpdatezK)(&z2zE776);
              KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE974);
              KILL(zTrieUpdate)(&zupdate);
              KILL(zTrieItemSink)(&znext_sink);
              KILL(zTrieChange)(&z2zE786);
              KILL(zTrieItemSink)(&z3zE978);
              KILL(zTrieItem)(&z2zE788);
              goto end_block_exception_1499;
            }
          }
          KILL(zTrieItem)(&z2zE788);
          goto finish_match_1494;
        }
      case_1495: ;
      finish_match_1494: ;
        COPY(zTrieItemSink)(&znext_sink, z3zE978);
        KILL(zTrieItemSink)(&z3zE978);
        KILL(zTrieChange)(&z2zE786);
      }
      struct zTrieUpdateCursor z2zE785;
      CREATE(zTrieUpdateCursor)(&z2zE785);
      ztrie_updates_advance(&z2zE785, zupdates);
      {
        zemit_updates_before_child(&z3zE974, znext_sink, z2zE785, zprefix, zchild);
        if (have_exception) {
          KILL(zoptionzIRTrieUpdatezK)(&z2zE776);
          KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE974);
          KILL(zTrieUpdate)(&zupdate);
          KILL(zTrieItemSink)(&znext_sink);
          KILL(zTrieUpdateCursor)(&z2zE785);
          goto end_block_exception_1499;
        }
      }
      KILL(zTrieUpdateCursor)(&z2zE785);
      KILL(zTrieItemSink)(&znext_sink);
    }
    KILL(zTrieUpdate)(&zupdate);
    goto finish_match_1492;
  }
case_1493: ;
finish_match_1492: ;
  COPY(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)((*(&z8zE599)), z3zE974);
  KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE974);
  KILL(zoptionzIRTrieUpdatezK)(&z2zE776);
end_function_1498: ;
  goto end_function_3591;
end_block_exception_1499: ;
  goto end_function_3591;
end_function_3591: ;
}

void zemit_leaf_overlay(struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 *z8zE600, struct zTrieItemSink zsink, struct zTrieUpdateCursor zupdates, struct zTriePath zprefix, struct zTriePath zkey, struct zByteSliceFields zvalue)
{
  struct zoptionzIRTrieUpdatezK z2zE755;
  CREATE(zoptionzIRTrieUpdatezK)(&z2zE755);
  z2zE755 = zupdates.zpending;
  struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z3zE961;
  CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE961);
  {
    if (z2zE755.kind != Kind_zNonezIRTrieUpdatezK) goto case_1489;
    struct zTrieItemSink z2zE757;
    CREATE(zTrieItemSink)(&z2zE757);
    {
      struct zTrieItem z2zE756;
      CREATE(zTrieItem)(&z2zE756);
      zitem_leaf(&z2zE756, zkey, zvalue);
      {
        ztrie_sink_emit(&z2zE757, zsink, z2zE756);
        if (have_exception) {
          KILL(zoptionzIRTrieUpdatezK)(&z2zE755);
          KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE961);
          KILL(zTrieItemSink)(&z2zE757);
          KILL(zTrieItem)(&z2zE756);
          goto end_block_exception_1491;
        }
      }
      KILL(zTrieItem)(&z2zE756);
    }
    COPY(zTrieItemSink)(&((&z3zE961)->ztup0), z2zE757);
    z3zE961.ztup1 = zupdates;
    KILL(zTrieItemSink)(&z2zE757);
    goto finish_match_1481;
  }
case_1489: ;
  {
    /* complete */
    struct zTrieUpdate zupdate;
    CREATE(zTrieUpdate)(&zupdate);
    zupdate = z2zE755.variants.zSomezIRTrieUpdatezK;
    bool z2zE760;
    {
      bool z2zE759;
      {
        struct zTriePath z2zE758;
        z2zE758 = zupdate.zkey;
        {
          z2zE759 = zpath_prefix_of(zprefix, z2zE758);
          if (have_exception) {
            KILL(zoptionzIRTrieUpdatezK)(&z2zE755);
            KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE961);
            KILL(zTrieUpdate)(&zupdate);
            goto end_block_exception_1491;
          }
        }
      }
      z2zE760 = not(z2zE759);
    }
    if (z2zE760) {
      struct zTrieItemSink z2zE762;
      CREATE(zTrieItemSink)(&z2zE762);
      {
        struct zTrieItem z2zE761;
        CREATE(zTrieItem)(&z2zE761);
        zitem_leaf(&z2zE761, zkey, zvalue);
        {
          ztrie_sink_emit(&z2zE762, zsink, z2zE761);
          if (have_exception) {
            KILL(zoptionzIRTrieUpdatezK)(&z2zE755);
            KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE961);
            KILL(zTrieUpdate)(&zupdate);
            KILL(zTrieItemSink)(&z2zE762);
            KILL(zTrieItem)(&z2zE761);
            goto end_block_exception_1491;
          }
        }
        KILL(zTrieItem)(&z2zE761);
      }
      struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z3zE969;
      CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE969);
      COPY(zTrieItemSink)(&((&z3zE969)->ztup0), z2zE762);
      z3zE969.ztup1 = zupdates;
      COPY(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE961, z3zE969);
      KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE969);
      KILL(zTrieItemSink)(&z2zE762);
    } else {
      bool z2zE764;
      {
        struct zTriePath z2zE763;
        z2zE763 = zupdate.zkey;
        z2zE764 = zpath_eq(z2zE763, zkey);
      }
      if (z2zE764) {
        struct zTrieItemSink zupdated_sink;
        CREATE(zTrieItemSink)(&zupdated_sink);
        {
          struct zTrieChange z2zE766;
          CREATE(zTrieChange)(&z2zE766);
          z2zE766 = zupdate.zchange;
          struct zTrieItemSink z3zE966;
          CREATE(zTrieItemSink)(&z3zE966);
          {
            if (z2zE766.kind != Kind_zTrieDelete) goto case_1488;
            COPY(zTrieItemSink)(&z3zE966, zsink);
            goto finish_match_1486;
          }
        case_1488: ;
          {
            /* complete */
            struct zByteSliceFields zupdated;
            zupdated = z2zE766.variants.zTriePut;
            struct zTrieItem z2zE767;
            CREATE(zTrieItem)(&z2zE767);
            zitem_leaf(&z2zE767, zkey, zupdated);
            {
              ztrie_sink_emit(&z3zE966, zsink, z2zE767);
              if (have_exception) {
                KILL(zoptionzIRTrieUpdatezK)(&z2zE755);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE961);
                KILL(zTrieUpdate)(&zupdate);
                KILL(zTrieItemSink)(&zupdated_sink);
                KILL(zTrieChange)(&z2zE766);
                KILL(zTrieItemSink)(&z3zE966);
                KILL(zTrieItem)(&z2zE767);
                goto end_block_exception_1491;
              }
            }
            KILL(zTrieItem)(&z2zE767);
            goto finish_match_1486;
          }
        case_1487: ;
        finish_match_1486: ;
          COPY(zTrieItemSink)(&zupdated_sink, z3zE966);
          KILL(zTrieItemSink)(&z3zE966);
          KILL(zTrieChange)(&z2zE766);
        }
        struct zTrieUpdateCursor z2zE765;
        CREATE(zTrieUpdateCursor)(&z2zE765);
        ztrie_updates_advance(&z2zE765, zupdates);
        {
          zemit_live_updates_under(&z3zE961, zupdated_sink, z2zE765, zprefix);
          if (have_exception) {
            KILL(zoptionzIRTrieUpdatezK)(&z2zE755);
            KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE961);
            KILL(zTrieUpdate)(&zupdate);
            KILL(zTrieItemSink)(&zupdated_sink);
            KILL(zTrieUpdateCursor)(&z2zE765);
            goto end_block_exception_1491;
          }
        }
        KILL(zTrieUpdateCursor)(&z2zE765);
        KILL(zTrieItemSink)(&zupdated_sink);
      } else {
        bool z2zE769;
        {
          struct zTriePath z2zE768;
          z2zE768 = zupdate.zkey;
          z2zE769 = zpath_lt(zkey, z2zE768);
        }
        if (z2zE769) {
          struct zTrieItemSink z2zE771;
          CREATE(zTrieItemSink)(&z2zE771);
          {
            struct zTrieItem z2zE770;
            CREATE(zTrieItem)(&z2zE770);
            zitem_leaf(&z2zE770, zkey, zvalue);
            {
              ztrie_sink_emit(&z2zE771, zsink, z2zE770);
              if (have_exception) {
                KILL(zoptionzIRTrieUpdatezK)(&z2zE755);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE961);
                KILL(zTrieUpdate)(&zupdate);
                KILL(zTrieItemSink)(&z2zE771);
                KILL(zTrieItem)(&z2zE770);
                goto end_block_exception_1491;
              }
            }
            KILL(zTrieItem)(&z2zE770);
          }
          {
            zemit_live_updates_under(&z3zE961, z2zE771, zupdates, zprefix);
            if (have_exception) {
              KILL(zoptionzIRTrieUpdatezK)(&z2zE755);
              KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE961);
              KILL(zTrieUpdate)(&zupdate);
              KILL(zTrieItemSink)(&z2zE771);
              goto end_block_exception_1491;
            }
          }
          KILL(zTrieItemSink)(&z2zE771);
        } else {
          struct zTrieItemSink z3zE972;
          CREATE(zTrieItemSink)(&z3zE972);
          {
            struct zTrieChange z2zE773;
            CREATE(zTrieChange)(&z2zE773);
            z2zE773 = zupdate.zchange;
            struct zTrieItemSink z3zE963;
            CREATE(zTrieItemSink)(&z3zE963);
            {
              if (z2zE773.kind != Kind_zTrieDelete) goto case_1485;
              COPY(zTrieItemSink)(&z3zE963, zsink);
              goto finish_match_1483;
            }
          case_1485: ;
            {
              /* complete */
              struct zByteSliceFields z3zE973;
              z3zE973 = z2zE773.variants.zTriePut;
              struct zTrieItem z2zE775;
              CREATE(zTrieItem)(&z2zE775);
              {
                struct zTriePath z2zE774;
                z2zE774 = zupdate.zkey;
                zitem_leaf(&z2zE775, z2zE774, z3zE973);
              }
              {
                ztrie_sink_emit(&z3zE963, zsink, z2zE775);
                if (have_exception) {
                  KILL(zoptionzIRTrieUpdatezK)(&z2zE755);
                  KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE961);
                  KILL(zTrieUpdate)(&zupdate);
                  KILL(zTrieItemSink)(&z3zE972);
                  KILL(zTrieChange)(&z2zE773);
                  KILL(zTrieItemSink)(&z3zE963);
                  KILL(zTrieItem)(&z2zE775);
                  goto end_block_exception_1491;
                }
              }
              KILL(zTrieItem)(&z2zE775);
              goto finish_match_1483;
            }
          case_1484: ;
          finish_match_1483: ;
            COPY(zTrieItemSink)(&z3zE972, z3zE963);
            KILL(zTrieItemSink)(&z3zE963);
            KILL(zTrieChange)(&z2zE773);
          }
          struct zTrieUpdateCursor z2zE772;
          CREATE(zTrieUpdateCursor)(&z2zE772);
          ztrie_updates_advance(&z2zE772, zupdates);
          {
            zemit_leaf_overlay(&z3zE961, z3zE972, z2zE772, zprefix, zkey, zvalue);
            if (have_exception) {
              KILL(zoptionzIRTrieUpdatezK)(&z2zE755);
              KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE961);
              KILL(zTrieUpdate)(&zupdate);
              KILL(zTrieItemSink)(&z3zE972);
              KILL(zTrieUpdateCursor)(&z2zE772);
              goto end_block_exception_1491;
            }
          }
          KILL(zTrieUpdateCursor)(&z2zE772);
          KILL(zTrieItemSink)(&z3zE972);
        }
      }
    }
    KILL(zTrieUpdate)(&zupdate);
    goto finish_match_1481;
  }
case_1482: ;
finish_match_1481: ;
  COPY(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)((*(&z8zE600)), z3zE961);
  KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE961);
  KILL(zoptionzIRTrieUpdatezK)(&z2zE755);
end_function_1490: ;
  goto end_function_3590;
end_block_exception_1491: ;
  goto end_function_3590;
end_function_3590: ;
}

void zwitness_emit(struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 *z8zE601, struct zByteSliceFields znode, struct zTriePath zprefix, struct zTrieUpdateCursor zupdates, struct zTrieItemSink zsink, uint64_t zcursor)
{
  bool z2zE734;
  {
    uint64_t z2zE733;
    z2zE733 = znode.zlen;
    z2zE734 = (z2zE733 == UINT64_C(0));
  }
  if (z2zE734) {
    {
      zemit_live_updates_under((*(&z8zE601)), zsink, zupdates, zprefix);
      if (have_exception) {  goto end_block_exception_1480;  }
    }
  } else {
    struct zTrieNode z2zE735;
    CREATE(zTrieNode)(&z2zE735);
    {
      zdecode_trie_node(&z2zE735, znode);
      if (have_exception) {
        KILL(zTrieNode)(&z2zE735);
        goto end_block_exception_1480;
      }
    }
    struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z3zE932;
    CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE932);
    {
      if (z2zE735.kind != Kind_zLeafNode) goto case_1478;
      struct zTriePath zpath;
      zpath = z2zE735.variants.zLeafNode.ztup0;
      struct zByteSliceFields zvalue;
      zvalue = z2zE735.variants.zLeafNode.ztup1;
      struct zTriePath zkey;
      {
        zkey = zpath_concat(zprefix, zpath);
        if (have_exception) {
          KILL(zTrieNode)(&z2zE735);
          KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE932);
          goto end_block_exception_1480;
        }
      }
      {
        zemit_leaf_overlay(&z3zE932, zsink, zupdates, zprefix, zkey, zvalue);
        if (have_exception) {
          KILL(zTrieNode)(&z2zE735);
          KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE932);
          goto end_block_exception_1480;
        }
      }
      goto finish_match_1464;
    }
  case_1478: ;
    {
      if (z2zE735.kind != Kind_zExtensionNode) goto case_1473;
      struct zTriePath z3zE956;
      z3zE956 = z2zE735.variants.zExtensionNode.ztup0;
      struct zNodeRef zchildref;
      CREATE(zNodeRef)(&zchildref);
      zchildref = z2zE735.variants.zExtensionNode.ztup1;
      uint64_t zextension_len;
      zextension_len = zpath_len(z3zE956);
      uint64_t znext_cursor;
      {    znext_cursor = (zcursor + zextension_len);
      }
      bool z2zE737;
      {
        bool z2zE736;
        z2zE736 = (zextension_len == UINT64_C(0));
        bool z3zE948;
        if (z2zE736) {  z3zE948 = true;  } else {  z3zE948 = (UINT64_C(64) < znext_cursor);  }
        z2zE737 = z3zE948;
      }
      if (z2zE737) {
        struct zexception z2zE738;
        CREATE(zexception)(&z2zE738);
        zInvalidBlock(&z2zE738, zWitnessDeficient);
        COPY(zexception)(current_exception, z2zE738);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/lib/mpt/trie.sail:143.20-143.56");
        KILL(zTrieNode)(&z2zE735);
        KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE932);
        KILL(zNodeRef)(&zchildref);
        KILL(zexception)(&z2zE738);
        goto end_block_exception_1480;
        /* unreachable after throw */
        KILL(zexception)(&z2zE738);
      } else {
        struct zTriePath zchild_prefix;
        {
          zchild_prefix = zpath_concat(zprefix, z3zE956);
          if (have_exception) {
            KILL(zTrieNode)(&z2zE735);
            KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE932);
            KILL(zNodeRef)(&zchildref);
            goto end_block_exception_1480;
          }
        }
        struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z2zE739;
        CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE739);
        {
          zemit_updates_before_child(&z2zE739, zsink, zupdates, zprefix, zchild_prefix);
          if (have_exception) {
            KILL(zTrieNode)(&z2zE735);
            KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE932);
            KILL(zNodeRef)(&zchildref);
            KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE739);
            goto end_block_exception_1480;
          }
        }
        struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z3zE950;
        CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE950);
        {
          struct zTrieItemSink zbefore_sink;
          CREATE(zTrieItemSink)(&zbefore_sink);
          COPY(zTrieItemSink)(&zbefore_sink, z2zE739.ztup0);
          struct zTrieUpdateCursor zchild_updates;
          CREATE(zTrieUpdateCursor)(&zchild_updates);
          zchild_updates = z2zE739.ztup1;
          struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z2zE745;
          CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE745);
          {
            bool z2zE740;
            {
              z2zE740 = znext_update_under(zchild_updates, zchild_prefix);
              if (have_exception) {
                KILL(zTrieNode)(&z2zE735);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE932);
                KILL(zNodeRef)(&zchildref);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE739);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE950);
                KILL(zTrieItemSink)(&zbefore_sink);
                KILL(zTrieUpdateCursor)(&zchild_updates);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE745);
                goto end_block_exception_1480;
              }
            }
            if (z2zE740) {
              struct zByteSliceFields zchild;
              {
                zchild = zresolve_ref(zchildref);
                if (have_exception) {
                  KILL(zTrieNode)(&z2zE735);
                  KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE932);
                  KILL(zNodeRef)(&zchildref);
                  KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE739);
                  KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE950);
                  KILL(zTrieItemSink)(&zbefore_sink);
                  KILL(zTrieUpdateCursor)(&zchild_updates);
                  KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE745);
                  goto end_block_exception_1480;
                }
              }
              bool z2zE742;
              {
                uint64_t z2zE741;
                z2zE741 = zchild.zlen;
                z2zE742 = (z2zE741 == UINT64_C(0));
              }
              if (z2zE742) {
                {
                  zemit_live_updates_under(&z2zE745, zbefore_sink, zchild_updates, zchild_prefix);
                  if (have_exception) {
                    KILL(zTrieNode)(&z2zE735);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE932);
                    KILL(zNodeRef)(&zchildref);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE739);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE950);
                    KILL(zTrieItemSink)(&zbefore_sink);
                    KILL(zTrieUpdateCursor)(&zchild_updates);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE745);
                    goto end_block_exception_1480;
                  }
                }
              } else {
                {
                  zwitness_emit(&z2zE745, zchild, zchild_prefix, zchild_updates, zbefore_sink, znext_cursor);
                  if (have_exception) {
                    KILL(zTrieNode)(&z2zE735);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE932);
                    KILL(zNodeRef)(&zchildref);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE739);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE950);
                    KILL(zTrieItemSink)(&zbefore_sink);
                    KILL(zTrieUpdateCursor)(&zchild_updates);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE745);
                    goto end_block_exception_1480;
                  }
                }
              }
            } else {
              struct zTrieItemSink z2zE744;
              CREATE(zTrieItemSink)(&z2zE744);
              {
                struct zTrieItem z2zE743;
                CREATE(zTrieItem)(&z2zE743);
                zitem_branch(&z2zE743, zchild_prefix, zchildref);
                {
                  ztrie_sink_emit(&z2zE744, zbefore_sink, z2zE743);
                  if (have_exception) {
                    KILL(zTrieNode)(&z2zE735);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE932);
                    KILL(zNodeRef)(&zchildref);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE739);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE950);
                    KILL(zTrieItemSink)(&zbefore_sink);
                    KILL(zTrieUpdateCursor)(&zchild_updates);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE745);
                    KILL(zTrieItemSink)(&z2zE744);
                    KILL(zTrieItem)(&z2zE743);
                    goto end_block_exception_1480;
                  }
                }
                KILL(zTrieItem)(&z2zE743);
              }
              struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z3zE951;
              CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE951);
              COPY(zTrieItemSink)(&((&z3zE951)->ztup0), z2zE744);
              z3zE951.ztup1 = zchild_updates;
              COPY(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE745, z3zE951);
              KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE951);
              KILL(zTrieItemSink)(&z2zE744);
            }
          }
          struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z3zE952;
          CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE952);
          {
            struct zTrieItemSink zchild_sink;
            CREATE(zTrieItemSink)(&zchild_sink);
            COPY(zTrieItemSink)(&zchild_sink, z2zE745.ztup0);
            struct zTrieUpdateCursor zlater_updates;
            CREATE(zTrieUpdateCursor)(&zlater_updates);
            zlater_updates = z2zE745.ztup1;
            {
              zemit_live_updates_under(&z3zE952, zchild_sink, zlater_updates, zprefix);
              if (have_exception) {
                KILL(zTrieNode)(&z2zE735);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE932);
                KILL(zNodeRef)(&zchildref);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE739);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE950);
                KILL(zTrieItemSink)(&zbefore_sink);
                KILL(zTrieUpdateCursor)(&zchild_updates);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE745);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE952);
                KILL(zTrieItemSink)(&zchild_sink);
                KILL(zTrieUpdateCursor)(&zlater_updates);
                goto end_block_exception_1480;
              }
            }
            KILL(zTrieUpdateCursor)(&zlater_updates);
            KILL(zTrieItemSink)(&zchild_sink);
            goto finish_match_1476;
          }
        case_1477: ;
          sail_match_failure("witness_emit");
        finish_match_1476: ;
          COPY(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE950, z3zE952);
          KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE952);
          KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE745);
          KILL(zTrieUpdateCursor)(&zchild_updates);
          KILL(zTrieItemSink)(&zbefore_sink);
          goto finish_match_1474;
        }
      case_1475: ;
        sail_match_failure("witness_emit");
      finish_match_1474: ;
        COPY(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE932, z3zE950);
        KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE950);
        KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE739);
      }
      KILL(zNodeRef)(&zchildref);
      goto finish_match_1464;
    }
  case_1473: ;
    {
      /* complete */
      zz5vecz8z5unionz0zzNodeRefz9 zchildren;
      CREATE(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren);
      COPY(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren, z2zE735.variants.zBranchNode.ztup0);
      struct zByteSliceFields z3zE957;
      z3zE957 = z2zE735.variants.zBranchNode.ztup1;
      bool z2zE748;
      {
        bool z2zE747;
        {
          uint64_t z2zE746;
          z2zE746 = z3zE957.zlen;
          z2zE747 = (z2zE746 != UINT64_C(0));
        }
        bool z3zE933;
        if (z2zE747) {  z3zE933 = true;  } else {  z3zE933 = (!(zcursor < UINT64_C(64)));  }
        z2zE748 = z3zE933;
      }
      if (z2zE748) {
        struct zexception z2zE749;
        CREATE(zexception)(&z2zE749);
        zInvalidBlock(&z2zE749, zWitnessDeficient);
        COPY(zexception)(current_exception, z2zE749);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/lib/mpt/trie.sail:162.20-162.56");
        KILL(zTrieNode)(&z2zE735);
        KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE932);
        KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren);
        KILL(zexception)(&z2zE749);
        goto end_block_exception_1480;
        /* unreachable after throw */
        KILL(zexception)(&z2zE749);
      } else {
        uint64_t z3zE958;
        {    z3zE958 = (zcursor + UINT64_C(1));
        }
        struct zTrieItemSink zcurrent_sink;
        CREATE(zTrieItemSink)(&zcurrent_sink);
        COPY(zTrieItemSink)(&zcurrent_sink, zsink);
        struct zTrieUpdateCursor zremaining;
        CREATE(zTrieUpdateCursor)(&zremaining);
        zremaining = zupdates;
        uint64_t znib;
        znib = UINT64_C(0x0);
        int64_t z3zE935;
        {    z3zE935 = (int64_t)(UINT64_C(0));
        }
        int64_t z3zE936;
        {    z3zE936 = (int64_t)(UINT64_C(15));
        }
        int64_t z3zE937;
        {    z3zE937 = (int64_t)(UINT64_C(1));
        }
        {
          int64_t zi;
          zi = z3zE935;
          unit z3zE945;
        for_start_1466: ;
          {
            if ((z3zE936 < zi)) goto for_end_1467;
            struct zTriePath z3zE959;
            {
              struct zTriePath z2zE754;
              {
                z2zE754 = zpath_single(znib);
                if (have_exception) {
                  KILL(zTrieNode)(&z2zE735);
                  KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE932);
                  KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren);
                  KILL(zTrieItemSink)(&zcurrent_sink);
                  KILL(zTrieUpdateCursor)(&zremaining);
                  goto end_block_exception_1480;
                }
              }
              {
                z3zE959 = zpath_concat(zprefix, z2zE754);
                if (have_exception) {
                  KILL(zTrieNode)(&z2zE735);
                  KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE932);
                  KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren);
                  KILL(zTrieItemSink)(&zcurrent_sink);
                  KILL(zTrieUpdateCursor)(&zremaining);
                  goto end_block_exception_1480;
                }
              }
            }
            struct zNodeRef z3zE960;
            CREATE(zNodeRef)(&z3zE960);
            fast_vector_access_zz5vecz8z5unionz0zzNodeRefz9(&z3zE960, zchildren, zi);
            bool zpresent;
            {
              bool z3zE938;
              {
                if (z3zE960.kind != Kind_zEmptyRef) goto case_1470;
                z3zE938 = false;
                goto finish_match_1468;
              }
            case_1470: ;
              {
                z3zE938 = true;
                goto finish_match_1468;
              }
            case_1469: ;
            finish_match_1468: ;
              zpresent = z3zE938;
            }
            bool z2zE750;
            {
              z2zE750 = znext_update_under(zremaining, z3zE959);
              if (have_exception) {
                KILL(zTrieNode)(&z2zE735);
                KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE932);
                KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren);
                KILL(zTrieItemSink)(&zcurrent_sink);
                KILL(zTrieUpdateCursor)(&zremaining);
                KILL(zNodeRef)(&z3zE960);
                goto end_block_exception_1480;
              }
            }
            unit z3zE941;
            if (z2zE750) {
              struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z2zE752;
              CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE752);
              if (zpresent) {
                struct zByteSliceFields z2zE751;
                {
                  z2zE751 = zresolve_ref(z3zE960);
                  if (have_exception) {
                    KILL(zTrieNode)(&z2zE735);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE932);
                    KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren);
                    KILL(zTrieItemSink)(&zcurrent_sink);
                    KILL(zTrieUpdateCursor)(&zremaining);
                    KILL(zNodeRef)(&z3zE960);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE752);
                    goto end_block_exception_1480;
                  }
                }
                {
                  zwitness_emit(&z2zE752, z2zE751, z3zE959, zremaining, zcurrent_sink, z3zE958);
                  if (have_exception) {
                    KILL(zTrieNode)(&z2zE735);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE932);
                    KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren);
                    KILL(zTrieItemSink)(&zcurrent_sink);
                    KILL(zTrieUpdateCursor)(&zremaining);
                    KILL(zNodeRef)(&z3zE960);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE752);
                    goto end_block_exception_1480;
                  }
                }
              } else {
                {
                  zemit_live_updates_under(&z2zE752, zcurrent_sink, zremaining, z3zE959);
                  if (have_exception) {
                    KILL(zTrieNode)(&z2zE735);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE932);
                    KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren);
                    KILL(zTrieItemSink)(&zcurrent_sink);
                    KILL(zTrieUpdateCursor)(&zremaining);
                    KILL(zNodeRef)(&z3zE960);
                    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE752);
                    goto end_block_exception_1480;
                  }
                }
              }
              unit z3zE942;
              {
                struct zTrieItemSink znext_sink;
                CREATE(zTrieItemSink)(&znext_sink);
                COPY(zTrieItemSink)(&znext_sink, z2zE752.ztup0);
                struct zTrieUpdateCursor znext_updates;
                CREATE(zTrieUpdateCursor)(&znext_updates);
                znext_updates = z2zE752.ztup1;
                COPY(zTrieItemSink)(&zcurrent_sink, znext_sink);
                unit z3zE943;
                z3zE943 = UNIT;
                zremaining = znext_updates;
                z3zE942 = UNIT;
                KILL(zTrieUpdateCursor)(&znext_updates);
                KILL(zTrieItemSink)(&znext_sink);
                goto finish_match_1471;
              }
            case_1472: ;
              sail_match_failure("witness_emit");
            finish_match_1471: ;
              z3zE941 = z3zE942;
              KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE752);
            } else if (zpresent) {
              struct zTrieItem z2zE753;
              CREATE(zTrieItem)(&z2zE753);
              zitem_subtree(&z2zE753, z3zE959, z3zE960);
              {
                ztrie_sink_emit(&zcurrent_sink, zcurrent_sink, z2zE753);
                if (have_exception) {
                  KILL(zTrieNode)(&z2zE735);
                  KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE932);
                  KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren);
                  KILL(zTrieItemSink)(&zcurrent_sink);
                  KILL(zTrieUpdateCursor)(&zremaining);
                  KILL(zNodeRef)(&z3zE960);
                  KILL(zTrieItem)(&z2zE753);
                  goto end_block_exception_1480;
                }
              }
              z3zE941 = UNIT;
              KILL(zTrieItem)(&z2zE753);
            } else {  z3zE941 = UNIT;  }
            znib = ((znib + UINT64_C(0x1)) & UINT64_C(0xF));
            z3zE945 = UNIT;
            KILL(zNodeRef)(&z3zE960);
            zi = (zi + z3zE937);
            goto for_start_1466;
          }
        for_end_1467: ;
        }
        unit z3zE946;
        z3zE946 = UNIT;
        struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z3zE947;
        CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE947);
        COPY(zTrieItemSink)(&((&z3zE947)->ztup0), zcurrent_sink);
        z3zE947.ztup1 = zremaining;
        COPY(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE932, z3zE947);
        KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE947);
        KILL(zTrieUpdateCursor)(&zremaining);
        KILL(zTrieItemSink)(&zcurrent_sink);
      }
      KILL(zz5vecz8z5unionz0zzNodeRefz9)(&zchildren);
      goto finish_match_1464;
    }
  case_1465: ;
  finish_match_1464: ;
    COPY(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)((*(&z8zE601)), z3zE932);
    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z3zE932);
    KILL(zTrieNode)(&z2zE735);
  }
end_function_1479: ;
  goto end_function_3589;
end_block_exception_1480: ;
  goto end_function_3589;
end_function_3589: ;
}

struct zTrieRootResult ztrie_root_cursor(sail_fixed_bytes_32 zbase_root, struct zTrieUpdateCursor zupdates)
{
  struct zTrieRootResult z8zE602;
  bool z2zE723;
  z2zE723 = zupdates_empty(zupdates);
  if (z2zE723) {
    struct zTrieRootResult z3zE931;
    z3zE931.zchanged = false;
    z3zE931.zroot = zbase_root;
    z8zE602 = z3zE931;
  } else {
    struct zTrieItemSink zsink;
    CREATE(zTrieItemSink)(&zsink);
    ztrie_sink_empty(&zsink, UNIT);
    struct ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9 z2zE728;
    CREATE(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE728);
    {
      bool z2zE724;
      z2zE724 = eq_fixed_bytes_32(zbase_root, zEMPTY_TRIE_ROOT);
      if (z2zE724) {
        struct zTriePath z3zE926;
        zz5vecz8z5bv8z9 z3zE927;
        CREATE(zz5vecz8z5bv8z9)(&z3zE927);
        internal_vector_init_zz5vecz8z5bv8z9(&z3zE927, INT64_C(32));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE927, z3zE927, INT64_C(0), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE927, z3zE927, INT64_C(1), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE927, z3zE927, INT64_C(2), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE927, z3zE927, INT64_C(3), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE927, z3zE927, INT64_C(4), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE927, z3zE927, INT64_C(5), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE927, z3zE927, INT64_C(6), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE927, z3zE927, INT64_C(7), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE927, z3zE927, INT64_C(8), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE927, z3zE927, INT64_C(9), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE927, z3zE927, INT64_C(10), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE927, z3zE927, INT64_C(11), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE927, z3zE927, INT64_C(12), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE927, z3zE927, INT64_C(13), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE927, z3zE927, INT64_C(14), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE927, z3zE927, INT64_C(15), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE927, z3zE927, INT64_C(16), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE927, z3zE927, INT64_C(17), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE927, z3zE927, INT64_C(18), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE927, z3zE927, INT64_C(19), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE927, z3zE927, INT64_C(20), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE927, z3zE927, INT64_C(21), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE927, z3zE927, INT64_C(22), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE927, z3zE927, INT64_C(23), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE927, z3zE927, INT64_C(24), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE927, z3zE927, INT64_C(25), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE927, z3zE927, INT64_C(26), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE927, z3zE927, INT64_C(27), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE927, z3zE927, INT64_C(28), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE927, z3zE927, INT64_C(29), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE927, z3zE927, INT64_C(30), UINT64_C(0x00));
        internal_vector_update_zz5vecz8z5bv8z9(&z3zE927, z3zE927, INT64_C(31), UINT64_C(0x00));
        for (size_t z8zE920 = 0; z8zE920 < 32; ++z8zE920) {
          z3zE926.zdata.bytes[z8zE920] = (uint8_t)(z3zE927.data[z8zE920] & UINT64_C(0xff));
        }
        KILL(zz5vecz8z5bv8z9)(&z3zE927);
        z3zE926.zlen = UINT64_C(0);
        {
          zemit_live_updates_under(&z2zE728, zsink, zupdates, z3zE926);
          if (have_exception) {
            KILL(zTrieItemSink)(&zsink);
            KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE728);
            goto end_block_exception_1463;
          }
        }
      } else {
        struct zByteSliceFields znode;
        znode = znode_db_lookup(zbase_root);
        bool z2zE726;
        {
          uint64_t z2zE725;
          z2zE725 = znode.zlen;
          z2zE726 = (z2zE725 == UINT64_C(0));
        }
        if (z2zE726) {
          struct zexception z2zE727;
          CREATE(zexception)(&z2zE727);
          zInvalidBlock(&z2zE727, zWitnessDeficient);
          COPY(zexception)(current_exception, z2zE727);
          have_exception = true;
          COPY(sail_string)(throw_location, "sail/lib/mpt/trie.sail:235.20-235.56");
          KILL(zTrieItemSink)(&zsink);
          KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE728);
          KILL(zexception)(&z2zE727);
          goto end_block_exception_1463;
          /* unreachable after throw */
          KILL(zexception)(&z2zE727);
        } else {
          struct zTriePath z3zE924;
          zz5vecz8z5bv8z9 z3zE925;
          CREATE(zz5vecz8z5bv8z9)(&z3zE925);
          internal_vector_init_zz5vecz8z5bv8z9(&z3zE925, INT64_C(32));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE925, z3zE925, INT64_C(0), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE925, z3zE925, INT64_C(1), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE925, z3zE925, INT64_C(2), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE925, z3zE925, INT64_C(3), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE925, z3zE925, INT64_C(4), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE925, z3zE925, INT64_C(5), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE925, z3zE925, INT64_C(6), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE925, z3zE925, INT64_C(7), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE925, z3zE925, INT64_C(8), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE925, z3zE925, INT64_C(9), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE925, z3zE925, INT64_C(10), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE925, z3zE925, INT64_C(11), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE925, z3zE925, INT64_C(12), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE925, z3zE925, INT64_C(13), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE925, z3zE925, INT64_C(14), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE925, z3zE925, INT64_C(15), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE925, z3zE925, INT64_C(16), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE925, z3zE925, INT64_C(17), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE925, z3zE925, INT64_C(18), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE925, z3zE925, INT64_C(19), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE925, z3zE925, INT64_C(20), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE925, z3zE925, INT64_C(21), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE925, z3zE925, INT64_C(22), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE925, z3zE925, INT64_C(23), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE925, z3zE925, INT64_C(24), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE925, z3zE925, INT64_C(25), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE925, z3zE925, INT64_C(26), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE925, z3zE925, INT64_C(27), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE925, z3zE925, INT64_C(28), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE925, z3zE925, INT64_C(29), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE925, z3zE925, INT64_C(30), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE925, z3zE925, INT64_C(31), UINT64_C(0x00));
          for (size_t z8zE919 = 0; z8zE919 < 32; ++z8zE919) {
            z3zE924.zdata.bytes[z8zE919] = (uint8_t)(z3zE925.data[z8zE919] & UINT64_C(0xff));
          }
          KILL(zz5vecz8z5bv8z9)(&z3zE925);
          z3zE924.zlen = UINT64_C(0);
          {
            zwitness_emit(&z2zE728, znode, z3zE924, zupdates, zsink, UINT64_C(0));
            if (have_exception) {
              KILL(zTrieItemSink)(&zsink);
              KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE728);
              goto end_block_exception_1463;
            }
          }
        }
      }
    }
    struct zTrieRootResult z3zE928;
    {
      struct zTrieItemSink zupdated_sink;
      CREATE(zTrieItemSink)(&zupdated_sink);
      COPY(zTrieItemSink)(&zupdated_sink, z2zE728.ztup0);
      struct zTrieUpdateCursor zremaining;
      CREATE(zTrieUpdateCursor)(&zremaining);
      zremaining = z2zE728.ztup1;
      bool z2zE729;
      z2zE729 = zupdates_empty(zremaining);
      if (z2zE729) {
        sail_fixed_bytes_32 z2zE731;
        {
          struct zTrieItemSink z2zE730;
          CREATE(zTrieItemSink)(&z2zE730);
          {
            ztrie_sink_finish(&z2zE730, zupdated_sink);
            if (have_exception) {
              KILL(zTrieItemSink)(&zsink);
              KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE728);
              KILL(zTrieItemSink)(&zupdated_sink);
              KILL(zTrieUpdateCursor)(&zremaining);
              KILL(zTrieItemSink)(&z2zE730);
              goto end_block_exception_1463;
            }
          }
          {
            z2zE731 = ztrie_sink_root(z2zE730);
            if (have_exception) {
              KILL(zTrieItemSink)(&zsink);
              KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE728);
              KILL(zTrieItemSink)(&zupdated_sink);
              KILL(zTrieUpdateCursor)(&zremaining);
              KILL(zTrieItemSink)(&z2zE730);
              goto end_block_exception_1463;
            }
          }
          KILL(zTrieItemSink)(&z2zE730);
        }
        struct zTrieRootResult z3zE930;
        z3zE930.zchanged = true;
        z3zE930.zroot = z2zE731;
        z3zE928 = z3zE930;
      } else {
        struct zexception z2zE732;
        CREATE(zexception)(&z2zE732);
        zInvalidBlock(&z2zE732, zWitnessDeficient);
        COPY(zexception)(current_exception, z2zE732);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/lib/mpt/trie.sail:243.12-243.48");
        KILL(zTrieItemSink)(&zsink);
        KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE728);
        KILL(zTrieItemSink)(&zupdated_sink);
        KILL(zTrieUpdateCursor)(&zremaining);
        KILL(zexception)(&z2zE732);
        goto end_block_exception_1463;
        /* unreachable after throw */
        KILL(zexception)(&z2zE732);
      }
      KILL(zTrieUpdateCursor)(&zremaining);
      KILL(zTrieItemSink)(&zupdated_sink);
      goto finish_match_1460;
    }
  case_1461: ;
    sail_match_failure("trie_root_cursor");
  finish_match_1460: ;
    z8zE602 = z3zE928;
    KILL(ztuple_z8z5structz0zzTrieItemSinkzCz0z5structz0zzTrieUpdateCursorz9)(&z2zE728);
    KILL(zTrieItemSink)(&zsink);
  }
end_function_1462: ;
  return z8zE602;
end_block_exception_1463: ;
  struct zTrieRootResult z8zE921 = { .zchanged = false, .zroot = fixed_bytes_32_zero() };
  return z8zE921;
}

struct zTrieRootResult ztrie_root(sail_fixed_bytes_32 zbase_root, struct zTrieUpdateSource zsource)
{
  struct zTrieRootResult z8zE603;
  struct zTrieUpdateCursor z2zE722;
  CREATE(zTrieUpdateCursor)(&z2zE722);
  ztrie_updates_begin(&z2zE722, zsource);
  {
    z8zE603 = ztrie_root_cursor(zbase_root, z2zE722);
    if (have_exception) {
      KILL(zTrieUpdateCursor)(&z2zE722);
      goto end_block_exception_1459;
    }
  }
  KILL(zTrieUpdateCursor)(&z2zE722);
end_function_1458: ;
  return z8zE603;
end_block_exception_1459: ;
  struct zTrieRootResult z8zE922 = { .zchanged = false, .zroot = fixed_bytes_32_zero() };
  return z8zE922;
}

