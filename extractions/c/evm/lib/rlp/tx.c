/* Generated from sail/lib/rlp/tx.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
void create_letbind_92(void) {    CREATE(zAccessListDecode)(&zEMPTY_ACCESS_LIST_DECODE);

  struct zAccessListDecode z3zE120;
  CREATE(zAccessListDecode)(&z3zE120);
  struct zAccessListDecode z3zE117;
  CREATE(zAccessListDecode)(&z3zE117);
  z3zE117.zaddress_count = UINT64_C(0);
  zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z3zE118;
  CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE118);
  COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE117)->zaddresses), z3zE118);
  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE118);
  z3zE117.zslot_count = UINT64_C(0);
  zz5listz8z5structz0zzStorageKeyz9 z3zE119;
  CREATE(zz5listz8z5structz0zzStorageKeyz9)(&z3zE119);
  COPY(zz5listz8z5structz0zzStorageKeyz9)(&((&z3zE117)->zstorage_slots), z3zE119);
  KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE119);
  COPY(zAccessListDecode)(&z3zE120, z3zE117);
  KILL(zAccessListDecode)(&z3zE117);
  COPY(zAccessListDecode)(&zEMPTY_ACCESS_LIST_DECODE, z3zE120);
  KILL(zAccessListDecode)(&z3zE120);
let_end_294: ;
}
void kill_letbind_92(void) {    KILL(zAccessListDecode)(&zEMPTY_ACCESS_LIST_DECODE);
}

void zdecode_access_list_keys(struct zAccessListDecode *z8zE189, struct zRlpCursor zcursor, sail_fixed_bytes_20 zaddr, struct zAccessListDecode ztail)
{
  bool z2zE3468;
  z2zE3468 = zrlp_cursor_empty(zcursor);
  if (z2zE3468) {
    zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE3469;
    CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE3469);
    COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE3469, ztail.zaddresses);
    zz5listz8z5structz0zzStorageKeyz9 z2zE3470;
    CREATE(zz5listz8z5structz0zzStorageKeyz9)(&z2zE3470);
    COPY(zz5listz8z5structz0zzStorageKeyz9)(&z2zE3470, ztail.zstorage_slots);
    uint64_t z2zE3471;
    z2zE3471 = ztail.zaddress_count;
    uint64_t z2zE3472;
    z2zE3472 = ztail.zslot_count;
    struct zAccessListDecode z3zE3412;
    CREATE(zAccessListDecode)(&z3zE3412);
    z3zE3412.zaddress_count = z2zE3471;
    COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE3412)->zaddresses), z2zE3469);
    z3zE3412.zslot_count = z2zE3472;
    COPY(zz5listz8z5structz0zzStorageKeyz9)(&((&z3zE3412)->zstorage_slots), z2zE3470);
    COPY(zAccessListDecode)((*(&z8zE189)), z3zE3412);
    KILL(zAccessListDecode)(&z3zE3412);
    KILL(zz5listz8z5structz0zzStorageKeyz9)(&z2zE3470);
    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE3469);
  } else {
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3473;
    {
      z2zE3473 = zrlp_cursor_pop(zcursor);
      if (have_exception) {  goto end_block_exception_3461;  }
    }
    struct zAccessListDecode z3zE3408;
    CREATE(zAccessListDecode)(&z3zE3408);
    {
      struct zRlpFieldRef zkey;
      zkey = z2zE3473.ztup0;
      struct zRlpCursor znext;
      znext = z2zE3473.ztup1;
      struct zStorageKey zstorage_key;
      {
        sail_u256 z2zE3480;
        {
          z2zE3480 = zrlp_ref_word(zkey);
          if (have_exception) {
            KILL(zAccessListDecode)(&z3zE3408);
            goto end_block_exception_3461;
          }
        }
        struct zStorageKey z3zE3409;
        z3zE3409.zaddr = zaddr;
        z3zE3409.zslot = z2zE3480;
        zstorage_key = z3zE3409;
      }
      struct zAccessListDecode zresult;
      CREATE(zAccessListDecode)(&zresult);
      {
        zdecode_access_list_keys(&zresult, znext, zaddr, ztail);
        if (have_exception) {
          KILL(zAccessListDecode)(&z3zE3408);
          KILL(zAccessListDecode)(&zresult);
          goto end_block_exception_3461;
        }
      }
      zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE3474;
      CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE3474);
      COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE3474, zresult.zaddresses);
      zz5listz8z5structz0zzStorageKeyz9 z2zE3476;
      CREATE(zz5listz8z5structz0zzStorageKeyz9)(&z2zE3476);
      {
        zz5listz8z5structz0zzStorageKeyz9 z2zE3475;
        CREATE(zz5listz8z5structz0zzStorageKeyz9)(&z2zE3475);
        COPY(zz5listz8z5structz0zzStorageKeyz9)(&z2zE3475, zresult.zstorage_slots);
        zconsz3z5structz0zzStorageKey(&z2zE3476, zstorage_key, z2zE3475);
        KILL(zz5listz8z5structz0zzStorageKeyz9)(&z2zE3475);
      }
      uint64_t z2zE3477;
      z2zE3477 = zresult.zaddress_count;
      uint64_t z2zE3479;
      {
        uint64_t z2zE3478;
        z2zE3478 = zresult.zslot_count;
        z2zE3479 = zrlp_item_count_incrementzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE3478);
      }
      struct zAccessListDecode z3zE3410;
      CREATE(zAccessListDecode)(&z3zE3410);
      z3zE3410.zaddress_count = z2zE3477;
      COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE3410)->zaddresses), z2zE3474);
      z3zE3410.zslot_count = z2zE3479;
      COPY(zz5listz8z5structz0zzStorageKeyz9)(&((&z3zE3410)->zstorage_slots), z2zE3476);
      COPY(zAccessListDecode)(&z3zE3408, z3zE3410);
      KILL(zAccessListDecode)(&z3zE3410);
      KILL(zz5listz8z5structz0zzStorageKeyz9)(&z2zE3476);
      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE3474);
      KILL(zAccessListDecode)(&zresult);
      goto finish_match_3458;
    }
  case_3459: ;
    sail_match_failure("decode_access_list_keys");
  finish_match_3458: ;
    COPY(zAccessListDecode)((*(&z8zE189)), z3zE3408);
    KILL(zAccessListDecode)(&z3zE3408);
  }
end_function_3460: ;
  goto end_function_4071;
end_block_exception_3461: ;
  goto end_function_4071;
end_function_4071: ;
}

void zdecode_access_list_entries(struct zAccessListDecode *z8zE190, struct zRlpCursor zcursor)
{
  bool z2zE3455;
  z2zE3455 = zrlp_cursor_empty(zcursor);
  if (z2zE3455) {
    struct zAccessListDecode z3zE3405;
    CREATE(zAccessListDecode)(&z3zE3405);
    z3zE3405.zaddress_count = UINT64_C(0);
    zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z3zE3406;
    CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3406);
    COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE3405)->zaddresses), z3zE3406);
    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3406);
    z3zE3405.zslot_count = UINT64_C(0);
    zz5listz8z5structz0zzStorageKeyz9 z3zE3407;
    CREATE(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3407);
    COPY(zz5listz8z5structz0zzStorageKeyz9)(&((&z3zE3405)->zstorage_slots), z3zE3407);
    KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3407);
    COPY(zAccessListDecode)((*(&z8zE190)), z3zE3405);
    KILL(zAccessListDecode)(&z3zE3405);
  } else {
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3456;
    {
      z2zE3456 = zrlp_cursor_pop(zcursor);
      if (have_exception) {  goto end_block_exception_3457;  }
    }
    struct zAccessListDecode z3zE3397;
    CREATE(zAccessListDecode)(&z3zE3397);
    {
      struct zRlpFieldRef zentry;
      zentry = z2zE3456.ztup0;
      struct zRlpCursor znext;
      znext = z2zE3456.ztup1;
      struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3458;
      {
        struct zRlpCursor z2zE3457;
        {
          z2zE3457 = zrlp_ref_cursor(zentry);
          if (have_exception) {
            KILL(zAccessListDecode)(&z3zE3397);
            goto end_block_exception_3457;
          }
        }
        {
          z2zE3458 = zrlp_cursor_pop(z2zE3457);
          if (have_exception) {
            KILL(zAccessListDecode)(&z3zE3397);
            goto end_block_exception_3457;
          }
        }
      }
      struct zAccessListDecode z3zE3398;
      CREATE(zAccessListDecode)(&z3zE3398);
      {
        struct zRlpFieldRef zaddr_f;
        zaddr_f = z2zE3458.ztup0;
        struct zRlpCursor zentry_fields;
        zentry_fields = z2zE3458.ztup1;
        struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3459;
        {
          z2zE3459 = zrlp_cursor_pop(zentry_fields);
          if (have_exception) {
            KILL(zAccessListDecode)(&z3zE3397);
            KILL(zAccessListDecode)(&z3zE3398);
            goto end_block_exception_3457;
          }
        }
        struct zAccessListDecode z3zE3399;
        CREATE(zAccessListDecode)(&z3zE3399);
        {
          struct zRlpFieldRef zkeys_f;
          zkeys_f = z2zE3459.ztup0;
          struct zRlpCursor z1zE73;
          z1zE73 = z2zE3459.ztup1;
          unit z3zE3400;
          {
            z3zE3400 = zrlp_cursor_expect_end(z1zE73);
            if (have_exception) {
              KILL(zAccessListDecode)(&z3zE3397);
              KILL(zAccessListDecode)(&z3zE3398);
              KILL(zAccessListDecode)(&z3zE3399);
              goto end_block_exception_3457;
            }
          }
          sail_fixed_bytes_20 zaddr;
          {
            sail_u256 z2zE3467;
            {
              z2zE3467 = zrlp_ref_word(zaddr_f);
              if (have_exception) {
                KILL(zAccessListDecode)(&z3zE3397);
                KILL(zAccessListDecode)(&z3zE3398);
                KILL(zAccessListDecode)(&z3zE3399);
                goto end_block_exception_3457;
              }
            }
            zaddr = zword_to_address(z2zE3467);
          }
          struct zAccessListDecode ztail;
          CREATE(zAccessListDecode)(&ztail);
          {
            zdecode_access_list_entries(&ztail, znext);
            if (have_exception) {
              KILL(zAccessListDecode)(&z3zE3397);
              KILL(zAccessListDecode)(&z3zE3398);
              KILL(zAccessListDecode)(&z3zE3399);
              KILL(zAccessListDecode)(&ztail);
              goto end_block_exception_3457;
            }
          }
          struct zAccessListDecode zresult;
          CREATE(zAccessListDecode)(&zresult);
          {
            struct zRlpCursor z2zE3466;
            {
              z2zE3466 = zrlp_ref_cursor(zkeys_f);
              if (have_exception) {
                KILL(zAccessListDecode)(&z3zE3397);
                KILL(zAccessListDecode)(&z3zE3398);
                KILL(zAccessListDecode)(&z3zE3399);
                KILL(zAccessListDecode)(&ztail);
                KILL(zAccessListDecode)(&zresult);
                goto end_block_exception_3457;
              }
            }
            {
              zdecode_access_list_keys(&zresult, z2zE3466, zaddr, ztail);
              if (have_exception) {
                KILL(zAccessListDecode)(&z3zE3397);
                KILL(zAccessListDecode)(&z3zE3398);
                KILL(zAccessListDecode)(&z3zE3399);
                KILL(zAccessListDecode)(&ztail);
                KILL(zAccessListDecode)(&zresult);
                goto end_block_exception_3457;
              }
            }
          }
          zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE3461;
          CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE3461);
          {
            zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE3460;
            CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE3460);
            COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE3460, zresult.zaddresses);
            zconsz3z5structz0zz__sail_c_repr_fixed_bytesz820z9(&z2zE3461, zaddr, z2zE3460);
            KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE3460);
          }
          zz5listz8z5structz0zzStorageKeyz9 z2zE3462;
          CREATE(zz5listz8z5structz0zzStorageKeyz9)(&z2zE3462);
          COPY(zz5listz8z5structz0zzStorageKeyz9)(&z2zE3462, zresult.zstorage_slots);
          uint64_t z2zE3464;
          {
            uint64_t z2zE3463;
            z2zE3463 = zresult.zaddress_count;
            z2zE3464 = zrlp_item_count_incrementzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE3463);
          }
          uint64_t z2zE3465;
          z2zE3465 = zresult.zslot_count;
          struct zAccessListDecode z3zE3401;
          CREATE(zAccessListDecode)(&z3zE3401);
          z3zE3401.zaddress_count = z2zE3464;
          COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE3401)->zaddresses), z2zE3461);
          z3zE3401.zslot_count = z2zE3465;
          COPY(zz5listz8z5structz0zzStorageKeyz9)(&((&z3zE3401)->zstorage_slots), z2zE3462);
          COPY(zAccessListDecode)(&z3zE3399, z3zE3401);
          KILL(zAccessListDecode)(&z3zE3401);
          KILL(zz5listz8z5structz0zzStorageKeyz9)(&z2zE3462);
          KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE3461);
          KILL(zAccessListDecode)(&zresult);
          KILL(zAccessListDecode)(&ztail);
          goto finish_match_3454;
        }
      case_3455: ;
        sail_match_failure("decode_access_list_entries");
      finish_match_3454: ;
        COPY(zAccessListDecode)(&z3zE3398, z3zE3399);
        KILL(zAccessListDecode)(&z3zE3399);
        goto finish_match_3452;
      }
    case_3453: ;
      sail_match_failure("decode_access_list_entries");
    finish_match_3452: ;
      COPY(zAccessListDecode)(&z3zE3397, z3zE3398);
      KILL(zAccessListDecode)(&z3zE3398);
      goto finish_match_3450;
    }
  case_3451: ;
    sail_match_failure("decode_access_list_entries");
  finish_match_3450: ;
    COPY(zAccessListDecode)((*(&z8zE190)), z3zE3397);
    KILL(zAccessListDecode)(&z3zE3397);
  }
end_function_3456: ;
  goto end_function_4070;
end_block_exception_3457: ;
  goto end_function_4070;
end_function_4070: ;
}

void zdecode_access_list(struct ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9 *z8zE191, struct zRlpFieldRef zf)
{
  bool z2zE3448;
  {
    uint64_t z2zE3447;
    {
      struct zByteSliceFields z2zE3446;
      z2zE3446 = zf.zsource;
      z2zE3447 = z2zE3446.zlen;
    }
    z2zE3448 = (!(UINT64_C(1073741824) < z2zE3447));
  }
  if (z2zE3448) {
    struct zAccessListDecode zdecoded;
    CREATE(zAccessListDecode)(&zdecoded);
    {
      struct zRlpCursor z2zE3453;
      {
        z2zE3453 = zrlp_ref_cursor(zf);
        if (have_exception) {
          KILL(zAccessListDecode)(&zdecoded);
          goto end_block_exception_3449;
        }
      }
      {
        zdecode_access_list_entries(&zdecoded, z2zE3453);
        if (have_exception) {
          KILL(zAccessListDecode)(&zdecoded);
          goto end_block_exception_3449;
        }
      }
    }
    zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE3449;
    CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE3449);
    COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE3449, zdecoded.zaddresses);
    zz5listz8z5structz0zzStorageKeyz9 z2zE3450;
    CREATE(zz5listz8z5structz0zzStorageKeyz9)(&z2zE3450);
    COPY(zz5listz8z5structz0zzStorageKeyz9)(&z2zE3450, zdecoded.zstorage_slots);
    uint64_t z2zE3451;
    z2zE3451 = zdecoded.zaddress_count;
    uint64_t z2zE3452;
    z2zE3452 = zdecoded.zslot_count;
    struct ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9 z3zE3396;
    CREATE(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z3zE3396);
    COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE3396)->ztup0), z2zE3449);
    COPY(zz5listz8z5structz0zzStorageKeyz9)(&((&z3zE3396)->ztup1), z2zE3450);
    z3zE3396.ztup2 = z2zE3451;
    z3zE3396.ztup3 = z2zE3452;
    COPY(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)((*(&z8zE191)), z3zE3396);
    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z3zE3396);
    KILL(zz5listz8z5structz0zzStorageKeyz9)(&z2zE3450);
    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE3449);
    KILL(zAccessListDecode)(&zdecoded);
  } else {
    struct zexception z2zE3454;
    CREATE(zexception)(&z2zE3454);
    zInvalidBlock(&z2zE3454, zRlpDecode);
    COPY(zexception)(current_exception, z2zE3454);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/tx.sail:116.8-116.37");
    KILL(zexception)(&z2zE3454);
    goto end_block_exception_3449;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3454);
  }
end_function_3448: ;
  goto end_function_4069;
end_block_exception_3449: ;
  goto end_function_4069;
end_function_4069: ;
}

void create_letbind_93(void) {

  uint64_t z3zE121;
  z3zE121 = UINT64_C(33);
  zBLOB_HASH_RLP_LENGTH = z3zE121;
let_end_299: ;
}
void kill_letbind_93(void) {
}

void create_letbind_94(void) {

  uint64_t z3zE122;
  z3zE122 = zWORD_BYTE_LENGTH;
  zBLOB_HASH_LENGTH = z3zE122;
let_end_300: ;
}
void kill_letbind_94(void) {
}

uint64_t zdecode_blob_hash_items(struct zRlpCursor zcursor, uint64_t zcount)
{
  uint64_t z8zE192;
  bool z2zE3431;
  z2zE3431 = zrlp_cursor_empty(zcursor);
  if (z2zE3431) {  z8zE192 = zcount;  } else {
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3432;
    {
      z2zE3432 = zrlp_cursor_pop(zcursor);
      if (have_exception) {  goto end_block_exception_3447;  }
    }
    uint64_t z3zE3390;
    {
      struct zRlpFieldRef zitem;
      zitem = z2zE3432.ztup0;
      struct zRlpCursor znext;
      znext = z2zE3432.ztup1;
      bool z2zE3441;
      {
        bool z2zE3440;
        z2zE3440 = zitem.zis_list;
        bool z3zE3393;
        if (z2zE3440) {  z3zE3393 = true;  } else {
          bool z2zE3439;
          {
            uint64_t z2zE3434;
            {
              struct zByteSliceFields z2zE3433;
              z2zE3433 = zitem.zsource;
              z2zE3434 = z2zE3433.zlen;
            }
            z2zE3439 = (z2zE3434 != zBLOB_HASH_RLP_LENGTH);
          }
          bool z3zE3392;
          if (z2zE3439) {  z3zE3392 = true;  } else {
            bool z2zE3438;
            {
              uint64_t z2zE3435;
              z2zE3435 = zitem.zcontent_len;
              z2zE3438 = (z2zE3435 != zBLOB_HASH_LENGTH);
            }
            bool z3zE3391;
            if (z2zE3438) {  z3zE3391 = true;  } else {
              uint64_t z2zE3437;
              {
                struct zByteSliceFields z2zE3436;
                z2zE3436 = zitem.zsource;
                z2zE3437 = zslice_byte(z2zE3436, UINT64_C(0));
              }
              z3zE3391 = (z2zE3437 != UINT64_C(0xA0));
            }
            z3zE3392 = z3zE3391;
          }
          z3zE3393 = z3zE3392;
        }
        z2zE3441 = z3zE3393;
      }
      unit z3zE3394;
      if (z2zE3441) {
        struct zexception z2zE3442;
        CREATE(zexception)(&z2zE3442);
        zInvalidBlock(&z2zE3442, zRlpDecode);
        COPY(zexception)(current_exception, z2zE3442);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/lib/rlp/tx.sail:150.12-150.41");
        KILL(zexception)(&z2zE3442);
        goto end_block_exception_3447;
        /* unreachable after throw */
        KILL(zexception)(&z2zE3442);
      } else {  z3zE3394 = UNIT;  }
      bool z2zE3443;
      z2zE3443 = (zcount < UINT64_C(9));
      if (z2zE3443) {
        uint64_t z2zE3444;
        {    z2zE3444 = (zcount + UINT64_C(1));
        }
        {
          z3zE3390 = zdecode_blob_hash_items(znext, z2zE3444);
          if (have_exception) {  goto end_block_exception_3447;  }
        }
      } else {
        struct zexception z2zE3445;
        CREATE(zexception)(&z2zE3445);
        zInvalidBlock(&z2zE3445, zRlpDecode);
        COPY(zexception)(current_exception, z2zE3445);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/lib/rlp/tx.sail:155.12-155.41");
        KILL(zexception)(&z2zE3445);
        goto end_block_exception_3447;
        /* unreachable after throw */
        KILL(zexception)(&z2zE3445);
      }
      goto finish_match_3444;
    }
  case_3445: ;
    sail_match_failure("decode_blob_hash_items");
  finish_match_3444: ;
    z8zE192 = z3zE3390;
  }
end_function_3446: ;
  return z8zE192;
end_block_exception_3447: ;

  return UINT64_C(0xdeadc0de);
}

struct zBlobHashes zdecode_blob_hashes(struct zRlpFieldRef zf)
{
  struct zBlobHashes z8zE193;
  struct zByteSliceFields zbytes;
  zbytes = zrlp_ref_content(zf);
  uint64_t z2zE3430;
  {
    struct zRlpCursor z2zE3429;
    {
      z2zE3429 = zrlp_ref_cursor(zf);
      if (have_exception) {  goto end_block_exception_3443;  }
    }
    {
      z2zE3430 = zdecode_blob_hash_items(z2zE3429, UINT64_C(0));
      if (have_exception) {  goto end_block_exception_3443;  }
    }
  }
  struct zBlobHashes z3zE3389;
  z3zE3389.zbytes = zbytes;
  z3zE3389.zcount = z2zE3430;
  z8zE193 = z3zE3389;
end_function_3442: ;
  return z8zE193;
end_block_exception_3443: ;
  struct zByteSliceFields z8zE850 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zBlobHashes z8zE849 = { .zbytes = z8zE850, .zcount = UINT64_C(0xdeadc0de) };
  return z8zE849;
}

void create_letbind_95(void) {    CREATE(zAuthorizzationDecode)(&zEMPTY_AUTHORIZATION_DECODE);

  struct zAuthorizzationDecode z3zE125;
  CREATE(zAuthorizzationDecode)(&z3zE125);
  struct zAuthorizzationDecode z3zE123;
  CREATE(zAuthorizzationDecode)(&z3zE123);
  zz5listz8z5structz0zzAuthorizzzzationz9 z3zE124;
  CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&z3zE124);
  COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&((&z3zE123)->zauthorizzations), z3zE124);
  KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z3zE124);
  z3zE123.zcount = UINT64_C(0);
  COPY(zAuthorizzationDecode)(&z3zE125, z3zE123);
  KILL(zAuthorizzationDecode)(&z3zE123);
  COPY(zAuthorizzationDecode)(&zEMPTY_AUTHORIZATION_DECODE, z3zE125);
  KILL(zAuthorizzationDecode)(&z3zE125);
let_end_303: ;
}
void kill_letbind_95(void) {    KILL(zAuthorizzationDecode)(&zEMPTY_AUTHORIZATION_DECODE);
}

void zdecode_auth_tuples(struct zAuthorizzationDecode *z8zE194, struct zRlpCursor zcursor)
{
  bool z2zE3407;
  z2zE3407 = zrlp_cursor_empty(zcursor);
  if (z2zE3407) {
    struct zAuthorizzationDecode z3zE3387;
    CREATE(zAuthorizzationDecode)(&z3zE3387);
    zz5listz8z5structz0zzAuthorizzzzationz9 z3zE3388;
    CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&z3zE3388);
    COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&((&z3zE3387)->zauthorizzations), z3zE3388);
    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z3zE3388);
    z3zE3387.zcount = UINT64_C(0);
    COPY(zAuthorizzationDecode)((*(&z8zE194)), z3zE3387);
    KILL(zAuthorizzationDecode)(&z3zE3387);
  } else {
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3408;
    {
      z2zE3408 = zrlp_cursor_pop(zcursor);
      if (have_exception) {  goto end_block_exception_3441;  }
    }
    struct zAuthorizzationDecode z3zE3361;
    CREATE(zAuthorizzationDecode)(&z3zE3361);
    {
      struct zRlpFieldRef ztuple;
      ztuple = z2zE3408.ztup0;
      struct zRlpCursor znext;
      znext = z2zE3408.ztup1;
      struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3410;
      {
        struct zRlpCursor z2zE3409;
        {
          z2zE3409 = zrlp_ref_cursor(ztuple);
          if (have_exception) {
            KILL(zAuthorizzationDecode)(&z3zE3361);
            goto end_block_exception_3441;
          }
        }
        {
          z2zE3410 = zrlp_cursor_pop(z2zE3409);
          if (have_exception) {
            KILL(zAuthorizzationDecode)(&z3zE3361);
            goto end_block_exception_3441;
          }
        }
      }
      struct zAuthorizzationDecode z3zE3362;
      CREATE(zAuthorizzationDecode)(&z3zE3362);
      {
        struct zRlpFieldRef zchain_f;
        zchain_f = z2zE3410.ztup0;
        struct zRlpCursor zfields;
        zfields = z2zE3410.ztup1;
        struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3411;
        {
          z2zE3411 = zrlp_cursor_pop(zfields);
          if (have_exception) {
            KILL(zAuthorizzationDecode)(&z3zE3361);
            KILL(zAuthorizzationDecode)(&z3zE3362);
            goto end_block_exception_3441;
          }
        }
        struct zAuthorizzationDecode z3zE3363;
        CREATE(zAuthorizzationDecode)(&z3zE3363);
        {
          struct zRlpFieldRef zaddr_f;
          zaddr_f = z2zE3411.ztup0;
          struct zRlpCursor z1zE68;
          z1zE68 = z2zE3411.ztup1;
          struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3412;
          {
            z2zE3412 = zrlp_cursor_pop(z1zE68);
            if (have_exception) {
              KILL(zAuthorizzationDecode)(&z3zE3361);
              KILL(zAuthorizzationDecode)(&z3zE3362);
              KILL(zAuthorizzationDecode)(&z3zE3363);
              goto end_block_exception_3441;
            }
          }
          struct zAuthorizzationDecode z3zE3364;
          CREATE(zAuthorizzationDecode)(&z3zE3364);
          {
            struct zRlpFieldRef znonce_f;
            znonce_f = z2zE3412.ztup0;
            struct zRlpCursor z1zE69;
            z1zE69 = z2zE3412.ztup1;
            struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3413;
            {
              z2zE3413 = zrlp_cursor_pop(z1zE69);
              if (have_exception) {
                KILL(zAuthorizzationDecode)(&z3zE3361);
                KILL(zAuthorizzationDecode)(&z3zE3362);
                KILL(zAuthorizzationDecode)(&z3zE3363);
                KILL(zAuthorizzationDecode)(&z3zE3364);
                goto end_block_exception_3441;
              }
            }
            struct zAuthorizzationDecode z3zE3365;
            CREATE(zAuthorizzationDecode)(&z3zE3365);
            {
              struct zRlpFieldRef zy_f;
              zy_f = z2zE3413.ztup0;
              struct zRlpCursor z1zE70;
              z1zE70 = z2zE3413.ztup1;
              struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3414;
              {
                z2zE3414 = zrlp_cursor_pop(z1zE70);
                if (have_exception) {
                  KILL(zAuthorizzationDecode)(&z3zE3361);
                  KILL(zAuthorizzationDecode)(&z3zE3362);
                  KILL(zAuthorizzationDecode)(&z3zE3363);
                  KILL(zAuthorizzationDecode)(&z3zE3364);
                  KILL(zAuthorizzationDecode)(&z3zE3365);
                  goto end_block_exception_3441;
                }
              }
              struct zAuthorizzationDecode z3zE3366;
              CREATE(zAuthorizzationDecode)(&z3zE3366);
              {
                struct zRlpFieldRef zr_f;
                zr_f = z2zE3414.ztup0;
                struct zRlpCursor z1zE71;
                z1zE71 = z2zE3414.ztup1;
                struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3415;
                {
                  z2zE3415 = zrlp_cursor_pop(z1zE71);
                  if (have_exception) {
                    KILL(zAuthorizzationDecode)(&z3zE3361);
                    KILL(zAuthorizzationDecode)(&z3zE3362);
                    KILL(zAuthorizzationDecode)(&z3zE3363);
                    KILL(zAuthorizzationDecode)(&z3zE3364);
                    KILL(zAuthorizzationDecode)(&z3zE3365);
                    KILL(zAuthorizzationDecode)(&z3zE3366);
                    goto end_block_exception_3441;
                  }
                }
                struct zAuthorizzationDecode z3zE3367;
                CREATE(zAuthorizzationDecode)(&z3zE3367);
                {
                  struct zRlpFieldRef zs_f;
                  zs_f = z2zE3415.ztup0;
                  struct zRlpCursor z1zE72;
                  z1zE72 = z2zE3415.ztup1;
                  unit z3zE3368;
                  {
                    z3zE3368 = zrlp_cursor_expect_end(z1zE72);
                    if (have_exception) {
                      KILL(zAuthorizzationDecode)(&z3zE3361);
                      KILL(zAuthorizzationDecode)(&z3zE3362);
                      KILL(zAuthorizzationDecode)(&z3zE3363);
                      KILL(zAuthorizzationDecode)(&z3zE3364);
                      KILL(zAuthorizzationDecode)(&z3zE3365);
                      KILL(zAuthorizzationDecode)(&z3zE3366);
                      KILL(zAuthorizzationDecode)(&z3zE3367);
                      goto end_block_exception_3441;
                    }
                  }
                  sail_u256 zchain_id;
                  {
                    zchain_id = zrlp_ref_uint_word(zchain_f);
                    if (have_exception) {
                      KILL(zAuthorizzationDecode)(&z3zE3361);
                      KILL(zAuthorizzationDecode)(&z3zE3362);
                      KILL(zAuthorizzationDecode)(&z3zE3363);
                      KILL(zAuthorizzationDecode)(&z3zE3364);
                      KILL(zAuthorizzationDecode)(&z3zE3365);
                      KILL(zAuthorizzationDecode)(&z3zE3366);
                      KILL(zAuthorizzationDecode)(&z3zE3367);
                      goto end_block_exception_3441;
                    }
                  }
                  uint64_t zauth_nonce;
                  {
                    zauth_nonce = zrlp_ref_uint64(znonce_f);
                    if (have_exception) {
                      KILL(zAuthorizzationDecode)(&z3zE3361);
                      KILL(zAuthorizzationDecode)(&z3zE3362);
                      KILL(zAuthorizzationDecode)(&z3zE3363);
                      KILL(zAuthorizzationDecode)(&z3zE3364);
                      KILL(zAuthorizzationDecode)(&z3zE3365);
                      KILL(zAuthorizzationDecode)(&z3zE3366);
                      KILL(zAuthorizzationDecode)(&z3zE3367);
                      goto end_block_exception_3441;
                    }
                  }
                  uint64_t zy_value;
                  {
                    zy_value = zrlp_ref_uint64(zy_f);
                    if (have_exception) {
                      KILL(zAuthorizzationDecode)(&z3zE3361);
                      KILL(zAuthorizzationDecode)(&z3zE3362);
                      KILL(zAuthorizzationDecode)(&z3zE3363);
                      KILL(zAuthorizzationDecode)(&z3zE3364);
                      KILL(zAuthorizzationDecode)(&z3zE3365);
                      KILL(zAuthorizzationDecode)(&z3zE3366);
                      KILL(zAuthorizzationDecode)(&z3zE3367);
                      goto end_block_exception_3441;
                    }
                  }
                  bool zy_valid;
                  zy_valid = (!(UINT64_C(1) < zy_value));
                  uint64_t zy;
                  {
                    bool z2zE3428;
                    z2zE3428 = (zy_value == UINT64_C(0));
                    if (z2zE3428) {  zy = UINT64_C(0);  } else {  zy = UINT64_C(1);  }
                  }
                  sail_u256 zr;
                  {
                    zr = zrlp_ref_uint_word(zr_f);
                    if (have_exception) {
                      KILL(zAuthorizzationDecode)(&z3zE3361);
                      KILL(zAuthorizzationDecode)(&z3zE3362);
                      KILL(zAuthorizzationDecode)(&z3zE3363);
                      KILL(zAuthorizzationDecode)(&z3zE3364);
                      KILL(zAuthorizzationDecode)(&z3zE3365);
                      KILL(zAuthorizzationDecode)(&z3zE3366);
                      KILL(zAuthorizzationDecode)(&z3zE3367);
                      goto end_block_exception_3441;
                    }
                  }
                  sail_u256 zs;
                  {
                    zs = zrlp_ref_uint_word(zs_f);
                    if (have_exception) {
                      KILL(zAuthorizzationDecode)(&z3zE3361);
                      KILL(zAuthorizzationDecode)(&z3zE3362);
                      KILL(zAuthorizzationDecode)(&z3zE3363);
                      KILL(zAuthorizzationDecode)(&z3zE3364);
                      KILL(zAuthorizzationDecode)(&z3zE3365);
                      KILL(zAuthorizzationDecode)(&z3zE3366);
                      KILL(zAuthorizzationDecode)(&z3zE3367);
                      goto end_block_exception_3441;
                    }
                  }
                  sail_fixed_bytes_20 zauth_addr;
                  {
                    sail_u256 z2zE3427;
                    {
                      z2zE3427 = zrlp_ref_word(zaddr_f);
                      if (have_exception) {
                        KILL(zAuthorizzationDecode)(&z3zE3361);
                        KILL(zAuthorizzationDecode)(&z3zE3362);
                        KILL(zAuthorizzationDecode)(&z3zE3363);
                        KILL(zAuthorizzationDecode)(&z3zE3364);
                        KILL(zAuthorizzationDecode)(&z3zE3365);
                        KILL(zAuthorizzationDecode)(&z3zE3366);
                        KILL(zAuthorizzationDecode)(&z3zE3367);
                        goto end_block_exception_3441;
                      }
                    }
                    zauth_addr = zword_to_address(z2zE3427);
                  }
                  struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE3417;
                  if (zy_valid) {
                    sail_fixed_bytes_32 z2zE3416;
                    z2zE3416 = zauth_signing_hash(zchain_id, zauth_addr, zauth_nonce);
                    z2zE3417 = zecrecover_addr(z2zE3416, zy, zr, zs);
                  } else {
                    struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z3zE3369;
                    z3zE3369.ztup0 = false;
                    z3zE3369.ztup1 = zZERO_ADDRESS;
                    z2zE3417 = z3zE3369;
                  }
                  struct zAuthorizzationDecode z3zE3370;
                  CREATE(zAuthorizzationDecode)(&z3zE3370);
                  {
                    bool zok;
                    zok = z2zE3417.ztup0;
                    sail_fixed_bytes_20 zauthority;
                    zauthority = z2zE3417.ztup1;
                    bool znonce_valid;
                    znonce_valid = (zauth_nonce != UINT64_C(18446744073709551615));
                    struct zAuthorizzation zauthorizzation;
                    {
                      bool z2zE3426;
                      {
                        bool z3zE3376;
                        if (zok) {
                          bool z3zE3375;
                          if (zy_valid) {
                            bool z2zE3425;
                            z2zE3425 = zword_ult(zZERO_WORD, zr);
                            bool z3zE3374;
                            if (z2zE3425) {
                              bool z2zE3424;
                              z2zE3424 = zword_ult(zr, zSECP_N_FULL);
                              bool z3zE3373;
                              if (z2zE3424) {
                                bool z2zE3423;
                                z2zE3423 = zword_ult(zZERO_WORD, zs);
                                bool z3zE3372;
                                if (z2zE3423) {
                                  bool z2zE3422;
                                  z2zE3422 = zword_ule(zs, zSECP_N_HALF);
                                  bool z3zE3371;
                                  if (z2zE3422) {  z3zE3371 = znonce_valid;  } else {  z3zE3371 = false;  }
                                  z3zE3372 = z3zE3371;
                                } else {  z3zE3372 = false;  }
                                z3zE3373 = z3zE3372;
                              } else {  z3zE3373 = false;  }
                              z3zE3374 = z3zE3373;
                            } else {  z3zE3374 = false;  }
                            z3zE3375 = z3zE3374;
                          } else {  z3zE3375 = false;  }
                          z3zE3376 = z3zE3375;
                        } else {  z3zE3376 = false;  }
                        z2zE3426 = z3zE3376;
                      }
                      struct zAuthorizzation z3zE3377;
                      z3zE3377.zaddress = zauth_addr;
                      z3zE3377.zauthority = zauthority;
                      z3zE3377.zchain_id = zchain_id;
                      z3zE3377.znonce = zauth_nonce;
                      z3zE3377.zvalid_sig = z2zE3426;
                      zauthorizzation = z3zE3377;
                    }
                    struct zAuthorizzationDecode ztail;
                    CREATE(zAuthorizzationDecode)(&ztail);
                    {
                      zdecode_auth_tuples(&ztail, znext);
                      if (have_exception) {
                        KILL(zAuthorizzationDecode)(&z3zE3361);
                        KILL(zAuthorizzationDecode)(&z3zE3362);
                        KILL(zAuthorizzationDecode)(&z3zE3363);
                        KILL(zAuthorizzationDecode)(&z3zE3364);
                        KILL(zAuthorizzationDecode)(&z3zE3365);
                        KILL(zAuthorizzationDecode)(&z3zE3366);
                        KILL(zAuthorizzationDecode)(&z3zE3367);
                        KILL(zAuthorizzationDecode)(&z3zE3370);
                        KILL(zAuthorizzationDecode)(&ztail);
                        goto end_block_exception_3441;
                      }
                    }
                    zz5listz8z5structz0zzAuthorizzzzationz9 z2zE3419;
                    CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE3419);
                    {
                      zz5listz8z5structz0zzAuthorizzzzationz9 z2zE3418;
                      CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE3418);
                      COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE3418, ztail.zauthorizzations);
                      zconsz3z5structz0zzAuthorizzzzation(&z2zE3419, zauthorizzation, z2zE3418);
                      KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE3418);
                    }
                    uint64_t z2zE3421;
                    {
                      uint64_t z2zE3420;
                      z2zE3420 = ztail.zcount;
                      z2zE3421 = zrlp_item_count_incrementzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE3420);
                    }
                    struct zAuthorizzationDecode z3zE3378;
                    CREATE(zAuthorizzationDecode)(&z3zE3378);
                    COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&((&z3zE3378)->zauthorizzations), z2zE3419);
                    z3zE3378.zcount = z2zE3421;
                    COPY(zAuthorizzationDecode)(&z3zE3370, z3zE3378);
                    KILL(zAuthorizzationDecode)(&z3zE3378);
                    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE3419);
                    KILL(zAuthorizzationDecode)(&ztail);
                    goto finish_match_3438;
                  }
                case_3439: ;
                  sail_match_failure("decode_auth_tuples");
                finish_match_3438: ;
                  COPY(zAuthorizzationDecode)(&z3zE3367, z3zE3370);
                  KILL(zAuthorizzationDecode)(&z3zE3370);
                  goto finish_match_3436;
                }
              case_3437: ;
                sail_match_failure("decode_auth_tuples");
              finish_match_3436: ;
                COPY(zAuthorizzationDecode)(&z3zE3366, z3zE3367);
                KILL(zAuthorizzationDecode)(&z3zE3367);
                goto finish_match_3434;
              }
            case_3435: ;
              sail_match_failure("decode_auth_tuples");
            finish_match_3434: ;
              COPY(zAuthorizzationDecode)(&z3zE3365, z3zE3366);
              KILL(zAuthorizzationDecode)(&z3zE3366);
              goto finish_match_3432;
            }
          case_3433: ;
            sail_match_failure("decode_auth_tuples");
          finish_match_3432: ;
            COPY(zAuthorizzationDecode)(&z3zE3364, z3zE3365);
            KILL(zAuthorizzationDecode)(&z3zE3365);
            goto finish_match_3430;
          }
        case_3431: ;
          sail_match_failure("decode_auth_tuples");
        finish_match_3430: ;
          COPY(zAuthorizzationDecode)(&z3zE3363, z3zE3364);
          KILL(zAuthorizzationDecode)(&z3zE3364);
          goto finish_match_3428;
        }
      case_3429: ;
        sail_match_failure("decode_auth_tuples");
      finish_match_3428: ;
        COPY(zAuthorizzationDecode)(&z3zE3362, z3zE3363);
        KILL(zAuthorizzationDecode)(&z3zE3363);
        goto finish_match_3426;
      }
    case_3427: ;
      sail_match_failure("decode_auth_tuples");
    finish_match_3426: ;
      COPY(zAuthorizzationDecode)(&z3zE3361, z3zE3362);
      KILL(zAuthorizzationDecode)(&z3zE3362);
      goto finish_match_3424;
    }
  case_3425: ;
    sail_match_failure("decode_auth_tuples");
  finish_match_3424: ;
    COPY(zAuthorizzationDecode)((*(&z8zE194)), z3zE3361);
    KILL(zAuthorizzationDecode)(&z3zE3361);
  }
end_function_3440: ;
  goto end_function_4068;
end_block_exception_3441: ;
  goto end_function_4068;
end_function_4068: ;
}

void zdecode_auth_list(struct ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9 *z8zE195, struct zRlpFieldRef zf)
{
  bool z2zE3402;
  {
    uint64_t z2zE3401;
    {
      struct zByteSliceFields z2zE3400;
      z2zE3400 = zf.zsource;
      z2zE3401 = z2zE3400.zlen;
    }
    z2zE3402 = (!(UINT64_C(1073741824) < z2zE3401));
  }
  if (z2zE3402) {
    struct zAuthorizzationDecode zdecoded;
    CREATE(zAuthorizzationDecode)(&zdecoded);
    {
      struct zRlpCursor z2zE3405;
      {
        z2zE3405 = zrlp_ref_cursor(zf);
        if (have_exception) {
          KILL(zAuthorizzationDecode)(&zdecoded);
          goto end_block_exception_3423;
        }
      }
      {
        zdecode_auth_tuples(&zdecoded, z2zE3405);
        if (have_exception) {
          KILL(zAuthorizzationDecode)(&zdecoded);
          goto end_block_exception_3423;
        }
      }
    }
    zz5listz8z5structz0zzAuthorizzzzationz9 z2zE3403;
    CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE3403);
    COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE3403, zdecoded.zauthorizzations);
    uint64_t z2zE3404;
    z2zE3404 = zdecoded.zcount;
    struct ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9 z3zE3360;
    CREATE(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z3zE3360);
    COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&((&z3zE3360)->ztup0), z2zE3403);
    z3zE3360.ztup1 = z2zE3404;
    COPY(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)((*(&z8zE195)), z3zE3360);
    KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z3zE3360);
    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE3403);
    KILL(zAuthorizzationDecode)(&zdecoded);
  } else {
    struct zexception z2zE3406;
    CREATE(zexception)(&z2zE3406);
    zInvalidBlock(&z2zE3406, zRlpDecode);
    COPY(zexception)(current_exception, z2zE3406);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/tx.sail:265.8-265.37");
    KILL(zexception)(&z2zE3406);
    goto end_block_exception_3423;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3406);
  }
end_function_3422: ;
  goto end_function_4067;
end_block_exception_3423: ;
  goto end_function_4067;
end_function_4067: ;
}

struct zByteSliceFields ztx_input_span(struct zRlpFieldRef zdata)
{
  struct zByteSliceFields z8zE196;
  struct zByteSliceFields zcontent;
  zcontent = zrlp_ref_content(zdata);
  bool z2zE3398;
  {
    uint64_t z2zE3397;
    z2zE3397 = zcontent.zlen;
    z2zE3398 = (!(UINT64_C(1073741824) < z2zE3397));
  }
  if (z2zE3398) {  z8zE196 = zcontent;  } else {
    struct zexception z2zE3399;
    CREATE(zexception)(&z2zE3399);
    zInvalidBlock(&z2zE3399, zRlpDecode);
    COPY(zexception)(current_exception, z2zE3399);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/tx.sail:280.8-280.37");
    KILL(zexception)(&z2zE3399);
    goto end_block_exception_3421;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3399);
  }
end_function_3420: ;
  return z8zE196;
end_block_exception_3421: ;
  struct zByteSliceFields z8zE851 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE851;
}

struct zByteSliceFields ztx_sig_span(struct zRlpFieldRef zfirst, struct zRlpFieldRef zsignature)
{
  struct zByteSliceFields z8zE197;
  uint64_t zstart;
  {
    struct zByteSliceFields z2zE3396;
    z2zE3396 = zfirst.zsource;
    zstart = z2zE3396.zoff;
  }
  uint64_t zstop;
  {
    struct zByteSliceFields z2zE3395;
    z2zE3395 = zsignature.zsource;
    zstop = z2zE3395.zoff;
  }
  uint64_t zstart_offset;
  zstart_offset = zstart;
  uint64_t zstop_offset;
  zstop_offset = zstop;
  bool z2zE3390;
  z2zE3390 = (zstop_offset < zstart_offset);
  enum zByteSource z2zE3393;
  {
    struct zByteSliceFields z2zE3392;
    z2zE3392 = zfirst.zsource;
    z2zE3393 = z2zE3392.zsource;
  }
  uint64_t z2zE3394;
  {    z2zE3394 = (zstop_offset - zstart_offset);
  }
  z8zE197 = zbyte_slice(z2zE3393, zstart_offset, z2zE3394);
end_function_3418: ;
  return z8zE197;
end_block_exception_3419: ;
  struct zByteSliceFields z8zE852 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE852;
}

uint64_t zrlp_ref_gas(struct zRlpFieldRef zf, enum zFork zfork)
{
  uint64_t z8zE198;
  uint64_t zvalue;
  {
    zvalue = zrlp_ref_uint64(zf);
    if (have_exception) {  goto end_block_exception_3417;  }
  }
  bool z2zE3388;
  {
    bool z2zE3387;
    z2zE3387 = zfork_gteq(zfork, zOsaka);
    bool z3zE3359;
    if (z2zE3387) {
      bool z2zE3386;
      z2zE3386 = zfork_lt(zfork, zAmsterdam);
      bool z3zE3358;
      if (z2zE3386) {  z3zE3358 = (zOSAKA_TRANSACTION_GAS_LIMIT_VALUE < zvalue);  } else {  z3zE3358 = false;  }
      z3zE3359 = z3zE3358;
    } else {  z3zE3359 = false;  }
    z2zE3388 = z3zE3359;
  }
  if (z2zE3388) {
    struct zexception z2zE3389;
    CREATE(zexception)(&z2zE3389);
    zInvalidBlock(&z2zE3389, zGasUsedExceedsLimit);
    COPY(zexception)(current_exception, z2zE3389);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/splices/c_optimized.sail:185.8-185.47");
    KILL(zexception)(&z2zE3389);
    goto end_block_exception_3417;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3389);
  } else {  z8zE198 = zvalue;  }
end_function_3416: ;
  return z8zE198;
end_block_exception_3417: ;

  return UINT64_C(0xdeadc0de);
}

void zdecode_legacy_tx(struct zTransaction *z8zE199, struct zByteSliceFields ztx, struct zByteSliceFields zpubkey, enum zFork zfork, sail_fixed_bytes_20 zsender, struct zRlpCursor zfields)
{
  struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3365;
  {
    z2zE3365 = zrlp_cursor_pop(zfields);
    if (have_exception) {  goto end_block_exception_3415;  }
  }
  struct zTransaction z3zE3335;
  CREATE(zTransaction)(&z3zE3335);
  {
    struct zRlpFieldRef znonce_f;
    znonce_f = z2zE3365.ztup0;
    struct zRlpCursor z1zE59;
    z1zE59 = z2zE3365.ztup1;
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3366;
    {
      z2zE3366 = zrlp_cursor_pop(z1zE59);
      if (have_exception) {
        KILL(zTransaction)(&z3zE3335);
        goto end_block_exception_3415;
      }
    }
    struct zTransaction z3zE3336;
    CREATE(zTransaction)(&z3zE3336);
    {
      struct zRlpFieldRef zgp_f;
      zgp_f = z2zE3366.ztup0;
      struct zRlpCursor z1zE60;
      z1zE60 = z2zE3366.ztup1;
      struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3367;
      {
        z2zE3367 = zrlp_cursor_pop(z1zE60);
        if (have_exception) {
          KILL(zTransaction)(&z3zE3335);
          KILL(zTransaction)(&z3zE3336);
          goto end_block_exception_3415;
        }
      }
      struct zTransaction z3zE3337;
      CREATE(zTransaction)(&z3zE3337);
      {
        struct zRlpFieldRef zgas_f;
        zgas_f = z2zE3367.ztup0;
        struct zRlpCursor z1zE61;
        z1zE61 = z2zE3367.ztup1;
        struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3368;
        {
          z2zE3368 = zrlp_cursor_pop(z1zE61);
          if (have_exception) {
            KILL(zTransaction)(&z3zE3335);
            KILL(zTransaction)(&z3zE3336);
            KILL(zTransaction)(&z3zE3337);
            goto end_block_exception_3415;
          }
        }
        struct zTransaction z3zE3338;
        CREATE(zTransaction)(&z3zE3338);
        {
          struct zRlpFieldRef zto_f;
          zto_f = z2zE3368.ztup0;
          struct zRlpCursor z1zE62;
          z1zE62 = z2zE3368.ztup1;
          struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3369;
          {
            z2zE3369 = zrlp_cursor_pop(z1zE62);
            if (have_exception) {
              KILL(zTransaction)(&z3zE3335);
              KILL(zTransaction)(&z3zE3336);
              KILL(zTransaction)(&z3zE3337);
              KILL(zTransaction)(&z3zE3338);
              goto end_block_exception_3415;
            }
          }
          struct zTransaction z3zE3339;
          CREATE(zTransaction)(&z3zE3339);
          {
            struct zRlpFieldRef zvalue_f;
            zvalue_f = z2zE3369.ztup0;
            struct zRlpCursor z1zE63;
            z1zE63 = z2zE3369.ztup1;
            struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3370;
            {
              z2zE3370 = zrlp_cursor_pop(z1zE63);
              if (have_exception) {
                KILL(zTransaction)(&z3zE3335);
                KILL(zTransaction)(&z3zE3336);
                KILL(zTransaction)(&z3zE3337);
                KILL(zTransaction)(&z3zE3338);
                KILL(zTransaction)(&z3zE3339);
                goto end_block_exception_3415;
              }
            }
            struct zTransaction z3zE3340;
            CREATE(zTransaction)(&z3zE3340);
            {
              struct zRlpFieldRef zdata_f;
              zdata_f = z2zE3370.ztup0;
              struct zRlpCursor z1zE64;
              z1zE64 = z2zE3370.ztup1;
              struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3371;
              {
                z2zE3371 = zrlp_cursor_pop(z1zE64);
                if (have_exception) {
                  KILL(zTransaction)(&z3zE3335);
                  KILL(zTransaction)(&z3zE3336);
                  KILL(zTransaction)(&z3zE3337);
                  KILL(zTransaction)(&z3zE3338);
                  KILL(zTransaction)(&z3zE3339);
                  KILL(zTransaction)(&z3zE3340);
                  goto end_block_exception_3415;
                }
              }
              struct zTransaction z3zE3341;
              CREATE(zTransaction)(&z3zE3341);
              {
                struct zRlpFieldRef zv_f;
                zv_f = z2zE3371.ztup0;
                struct zRlpCursor z1zE65;
                z1zE65 = z2zE3371.ztup1;
                struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3372;
                {
                  z2zE3372 = zrlp_cursor_pop(z1zE65);
                  if (have_exception) {
                    KILL(zTransaction)(&z3zE3335);
                    KILL(zTransaction)(&z3zE3336);
                    KILL(zTransaction)(&z3zE3337);
                    KILL(zTransaction)(&z3zE3338);
                    KILL(zTransaction)(&z3zE3339);
                    KILL(zTransaction)(&z3zE3340);
                    KILL(zTransaction)(&z3zE3341);
                    goto end_block_exception_3415;
                  }
                }
                struct zTransaction z3zE3342;
                CREATE(zTransaction)(&z3zE3342);
                {
                  struct zRlpFieldRef zr_f;
                  zr_f = z2zE3372.ztup0;
                  struct zRlpCursor z1zE66;
                  z1zE66 = z2zE3372.ztup1;
                  struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3373;
                  {
                    z2zE3373 = zrlp_cursor_pop(z1zE66);
                    if (have_exception) {
                      KILL(zTransaction)(&z3zE3335);
                      KILL(zTransaction)(&z3zE3336);
                      KILL(zTransaction)(&z3zE3337);
                      KILL(zTransaction)(&z3zE3338);
                      KILL(zTransaction)(&z3zE3339);
                      KILL(zTransaction)(&z3zE3340);
                      KILL(zTransaction)(&z3zE3341);
                      KILL(zTransaction)(&z3zE3342);
                      goto end_block_exception_3415;
                    }
                  }
                  struct zTransaction z3zE3343;
                  CREATE(zTransaction)(&z3zE3343);
                  {
                    struct zRlpFieldRef zs_f;
                    zs_f = z2zE3373.ztup0;
                    struct zRlpCursor z1zE67;
                    z1zE67 = z2zE3373.ztup1;
                    unit z3zE3344;
                    {
                      z3zE3344 = zrlp_cursor_expect_end(z1zE67);
                      if (have_exception) {
                        KILL(zTransaction)(&z3zE3335);
                        KILL(zTransaction)(&z3zE3336);
                        KILL(zTransaction)(&z3zE3337);
                        KILL(zTransaction)(&z3zE3338);
                        KILL(zTransaction)(&z3zE3339);
                        KILL(zTransaction)(&z3zE3340);
                        KILL(zTransaction)(&z3zE3341);
                        KILL(zTransaction)(&z3zE3342);
                        KILL(zTransaction)(&z3zE3343);
                        goto end_block_exception_3415;
                      }
                    }
                    sail_u256 zv;
                    {
                      zv = zrlp_ref_word(zv_f);
                      if (have_exception) {
                        KILL(zTransaction)(&z3zE3335);
                        KILL(zTransaction)(&z3zE3336);
                        KILL(zTransaction)(&z3zE3337);
                        KILL(zTransaction)(&z3zE3338);
                        KILL(zTransaction)(&z3zE3339);
                        KILL(zTransaction)(&z3zE3340);
                        KILL(zTransaction)(&z3zE3341);
                        KILL(zTransaction)(&z3zE3342);
                        KILL(zTransaction)(&z3zE3343);
                        goto end_block_exception_3415;
                      }
                    }
                    sail_u256 zgp;
                    {
                      zgp = zrlp_ref_uint_word(zgp_f);
                      if (have_exception) {
                        KILL(zTransaction)(&z3zE3335);
                        KILL(zTransaction)(&z3zE3336);
                        KILL(zTransaction)(&z3zE3337);
                        KILL(zTransaction)(&z3zE3338);
                        KILL(zTransaction)(&z3zE3339);
                        KILL(zTransaction)(&z3zE3340);
                        KILL(zTransaction)(&z3zE3341);
                        KILL(zTransaction)(&z3zE3342);
                        KILL(zTransaction)(&z3zE3343);
                        goto end_block_exception_3415;
                      }
                    }
                    sail_u256 z2zE3374;
                    {
                      z2zE3374 = zrlp_ref_uint_word(znonce_f);
                      if (have_exception) {
                        KILL(zTransaction)(&z3zE3335);
                        KILL(zTransaction)(&z3zE3336);
                        KILL(zTransaction)(&z3zE3337);
                        KILL(zTransaction)(&z3zE3338);
                        KILL(zTransaction)(&z3zE3339);
                        KILL(zTransaction)(&z3zE3340);
                        KILL(zTransaction)(&z3zE3341);
                        KILL(zTransaction)(&z3zE3342);
                        KILL(zTransaction)(&z3zE3343);
                        goto end_block_exception_3415;
                      }
                    }
                    uint64_t z2zE3375;
                    {
                      z2zE3375 = zrlp_ref_gas(zgas_f, zfork);
                      if (have_exception) {
                        KILL(zTransaction)(&z3zE3335);
                        KILL(zTransaction)(&z3zE3336);
                        KILL(zTransaction)(&z3zE3337);
                        KILL(zTransaction)(&z3zE3338);
                        KILL(zTransaction)(&z3zE3339);
                        KILL(zTransaction)(&z3zE3340);
                        KILL(zTransaction)(&z3zE3341);
                        KILL(zTransaction)(&z3zE3342);
                        KILL(zTransaction)(&z3zE3343);
                        goto end_block_exception_3415;
                      }
                    }
                    bool z2zE3377;
                    {
                      uint64_t z2zE3376;
                      z2zE3376 = zto_f.zcontent_len;
                      z2zE3377 = (z2zE3376 == UINT64_C(0));
                    }
                    sail_fixed_bytes_20 z2zE3379;
                    {
                      sail_u256 z2zE3378;
                      {
                        z2zE3378 = zrlp_ref_word(zto_f);
                        if (have_exception) {
                          KILL(zTransaction)(&z3zE3335);
                          KILL(zTransaction)(&z3zE3336);
                          KILL(zTransaction)(&z3zE3337);
                          KILL(zTransaction)(&z3zE3338);
                          KILL(zTransaction)(&z3zE3339);
                          KILL(zTransaction)(&z3zE3340);
                          KILL(zTransaction)(&z3zE3341);
                          KILL(zTransaction)(&z3zE3342);
                          KILL(zTransaction)(&z3zE3343);
                          goto end_block_exception_3415;
                        }
                      }
                      z2zE3379 = zword_to_address(z2zE3378);
                    }
                    sail_u256 z2zE3380;
                    {
                      z2zE3380 = zrlp_ref_uint_word(zvalue_f);
                      if (have_exception) {
                        KILL(zTransaction)(&z3zE3335);
                        KILL(zTransaction)(&z3zE3336);
                        KILL(zTransaction)(&z3zE3337);
                        KILL(zTransaction)(&z3zE3338);
                        KILL(zTransaction)(&z3zE3339);
                        KILL(zTransaction)(&z3zE3340);
                        KILL(zTransaction)(&z3zE3341);
                        KILL(zTransaction)(&z3zE3342);
                        KILL(zTransaction)(&z3zE3343);
                        goto end_block_exception_3415;
                      }
                    }
                    struct zByteSliceFields z2zE3381;
                    {
                      z2zE3381 = ztx_input_span(zdata_f);
                      if (have_exception) {
                        KILL(zTransaction)(&z3zE3335);
                        KILL(zTransaction)(&z3zE3336);
                        KILL(zTransaction)(&z3zE3337);
                        KILL(zTransaction)(&z3zE3338);
                        KILL(zTransaction)(&z3zE3339);
                        KILL(zTransaction)(&z3zE3340);
                        KILL(zTransaction)(&z3zE3341);
                        KILL(zTransaction)(&z3zE3342);
                        KILL(zTransaction)(&z3zE3343);
                        goto end_block_exception_3415;
                      }
                    }
                    sail_fixed_bytes_32 z2zE3383;
                    {
                      struct zByteSliceFields z2zE3382;
                      {
                        z2zE3382 = ztx_sig_span(znonce_f, zv_f);
                        if (have_exception) {
                          KILL(zTransaction)(&z3zE3335);
                          KILL(zTransaction)(&z3zE3336);
                          KILL(zTransaction)(&z3zE3337);
                          KILL(zTransaction)(&z3zE3338);
                          KILL(zTransaction)(&z3zE3339);
                          KILL(zTransaction)(&z3zE3340);
                          KILL(zTransaction)(&z3zE3341);
                          KILL(zTransaction)(&z3zE3342);
                          KILL(zTransaction)(&z3zE3343);
                          goto end_block_exception_3415;
                        }
                      }
                      {
                        z2zE3383 = ztx_signing_hash(zLegacyTx, z2zE3382, zv);
                        if (have_exception) {
                          KILL(zTransaction)(&z3zE3335);
                          KILL(zTransaction)(&z3zE3336);
                          KILL(zTransaction)(&z3zE3337);
                          KILL(zTransaction)(&z3zE3338);
                          KILL(zTransaction)(&z3zE3339);
                          KILL(zTransaction)(&z3zE3340);
                          KILL(zTransaction)(&z3zE3341);
                          KILL(zTransaction)(&z3zE3342);
                          KILL(zTransaction)(&z3zE3343);
                          goto end_block_exception_3415;
                        }
                      }
                    }
                    sail_u256 z2zE3384;
                    {
                      z2zE3384 = zrlp_ref_uint_word(zr_f);
                      if (have_exception) {
                        KILL(zTransaction)(&z3zE3335);
                        KILL(zTransaction)(&z3zE3336);
                        KILL(zTransaction)(&z3zE3337);
                        KILL(zTransaction)(&z3zE3338);
                        KILL(zTransaction)(&z3zE3339);
                        KILL(zTransaction)(&z3zE3340);
                        KILL(zTransaction)(&z3zE3341);
                        KILL(zTransaction)(&z3zE3342);
                        KILL(zTransaction)(&z3zE3343);
                        goto end_block_exception_3415;
                      }
                    }
                    sail_u256 z2zE3385;
                    {
                      z2zE3385 = zrlp_ref_uint_word(zs_f);
                      if (have_exception) {
                        KILL(zTransaction)(&z3zE3335);
                        KILL(zTransaction)(&z3zE3336);
                        KILL(zTransaction)(&z3zE3337);
                        KILL(zTransaction)(&z3zE3338);
                        KILL(zTransaction)(&z3zE3339);
                        KILL(zTransaction)(&z3zE3340);
                        KILL(zTransaction)(&z3zE3341);
                        KILL(zTransaction)(&z3zE3342);
                        KILL(zTransaction)(&z3zE3343);
                        goto end_block_exception_3415;
                      }
                    }
                    z3zE3343.zaccess_list_address_count = UINT64_C(0);
                    zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z3zE3346;
                    CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3346);
                    COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE3343)->zaccess_list_addresses), z3zE3346);
                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3346);
                    z3zE3343.zaccess_list_slot_count = UINT64_C(0);
                    zz5listz8z5structz0zzStorageKeyz9 z3zE3347;
                    CREATE(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3347);
                    COPY(zz5listz8z5structz0zzStorageKeyz9)(&((&z3zE3343)->zaccess_list_slots), z3zE3347);
                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3347);
                    z3zE3343.zauthorizzation_count = UINT64_C(0);
                    zz5listz8z5structz0zzAuthorizzzzationz9 z3zE3348;
                    CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&z3zE3348);
                    COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&((&z3zE3343)->zauthorizzations), z3zE3348);
                    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z3zE3348);
                    z3zE3343.zblob_hashes = zEMPTY_BLOB_HASHES;
                    z3zE3343.zchain_id = UINT64_C(0);
                    z3zE3343.zgas_limit = z2zE3375;
                    z3zE3343.zinput_src = z2zE3381;
                    z3zE3343.zis_create = z2zE3377;
                    z3zE3343.zmax_blob_fee = zZERO_WORD;
                    z3zE3343.zmax_fee = zgp;
                    z3zE3343.zmax_priority_fee = zgp;
                    z3zE3343.znonce = z2zE3374;
                    z3zE3343.zpubkey = zpubkey;
                    z3zE3343.zraw = ztx;
                    z3zE3343.zrecipient = z2zE3379;
                    z3zE3343.zsender = zsender;
                    z3zE3343.zsig_r = z2zE3384;
                    z3zE3343.zsig_s = z2zE3385;
                    z3zE3343.zsig_v = zv;
                    z3zE3343.zsigning_hash = z2zE3383;
                    z3zE3343.ztx_type = zLegacyTx;
                    z3zE3343.zvalue = z2zE3380;
                    goto finish_match_3412;
                  }
                case_3413: ;
                  sail_match_failure("decode_legacy_tx");
                finish_match_3412: ;
                  COPY(zTransaction)(&z3zE3342, z3zE3343);
                  KILL(zTransaction)(&z3zE3343);
                  goto finish_match_3410;
                }
              case_3411: ;
                sail_match_failure("decode_legacy_tx");
              finish_match_3410: ;
                COPY(zTransaction)(&z3zE3341, z3zE3342);
                KILL(zTransaction)(&z3zE3342);
                goto finish_match_3408;
              }
            case_3409: ;
              sail_match_failure("decode_legacy_tx");
            finish_match_3408: ;
              COPY(zTransaction)(&z3zE3340, z3zE3341);
              KILL(zTransaction)(&z3zE3341);
              goto finish_match_3406;
            }
          case_3407: ;
            sail_match_failure("decode_legacy_tx");
          finish_match_3406: ;
            COPY(zTransaction)(&z3zE3339, z3zE3340);
            KILL(zTransaction)(&z3zE3340);
            goto finish_match_3404;
          }
        case_3405: ;
          sail_match_failure("decode_legacy_tx");
        finish_match_3404: ;
          COPY(zTransaction)(&z3zE3338, z3zE3339);
          KILL(zTransaction)(&z3zE3339);
          goto finish_match_3402;
        }
      case_3403: ;
        sail_match_failure("decode_legacy_tx");
      finish_match_3402: ;
        COPY(zTransaction)(&z3zE3337, z3zE3338);
        KILL(zTransaction)(&z3zE3338);
        goto finish_match_3400;
      }
    case_3401: ;
      sail_match_failure("decode_legacy_tx");
    finish_match_3400: ;
      COPY(zTransaction)(&z3zE3336, z3zE3337);
      KILL(zTransaction)(&z3zE3337);
      goto finish_match_3398;
    }
  case_3399: ;
    sail_match_failure("decode_legacy_tx");
  finish_match_3398: ;
    COPY(zTransaction)(&z3zE3335, z3zE3336);
    KILL(zTransaction)(&z3zE3336);
    goto finish_match_3396;
  }
case_3397: ;
  sail_match_failure("decode_legacy_tx");
finish_match_3396: ;
  COPY(zTransaction)((*(&z8zE199)), z3zE3335);
  KILL(zTransaction)(&z3zE3335);
end_function_3414: ;
  goto end_function_4066;
end_block_exception_3415: ;
  goto end_function_4066;
end_function_4066: ;
}

void zdecode_access_list_tx(struct zTransaction *z8zE200, struct zByteSliceFields ztx, struct zByteSliceFields zpubkey, enum zFork zfork, sail_fixed_bytes_20 zsender, struct zRlpCursor zfields)
{
  struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3340;
  {
    z2zE3340 = zrlp_cursor_pop(zfields);
    if (have_exception) {  goto end_block_exception_3395;  }
  }
  struct zTransaction z3zE3308;
  CREATE(zTransaction)(&z3zE3308);
  {
    struct zRlpFieldRef zchain_f;
    zchain_f = z2zE3340.ztup0;
    struct zRlpCursor z1zE48;
    z1zE48 = z2zE3340.ztup1;
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3341;
    {
      z2zE3341 = zrlp_cursor_pop(z1zE48);
      if (have_exception) {
        KILL(zTransaction)(&z3zE3308);
        goto end_block_exception_3395;
      }
    }
    struct zTransaction z3zE3309;
    CREATE(zTransaction)(&z3zE3309);
    {
      struct zRlpFieldRef znonce_f;
      znonce_f = z2zE3341.ztup0;
      struct zRlpCursor z1zE49;
      z1zE49 = z2zE3341.ztup1;
      struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3342;
      {
        z2zE3342 = zrlp_cursor_pop(z1zE49);
        if (have_exception) {
          KILL(zTransaction)(&z3zE3308);
          KILL(zTransaction)(&z3zE3309);
          goto end_block_exception_3395;
        }
      }
      struct zTransaction z3zE3310;
      CREATE(zTransaction)(&z3zE3310);
      {
        struct zRlpFieldRef zgp_f;
        zgp_f = z2zE3342.ztup0;
        struct zRlpCursor z1zE50;
        z1zE50 = z2zE3342.ztup1;
        struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3343;
        {
          z2zE3343 = zrlp_cursor_pop(z1zE50);
          if (have_exception) {
            KILL(zTransaction)(&z3zE3308);
            KILL(zTransaction)(&z3zE3309);
            KILL(zTransaction)(&z3zE3310);
            goto end_block_exception_3395;
          }
        }
        struct zTransaction z3zE3311;
        CREATE(zTransaction)(&z3zE3311);
        {
          struct zRlpFieldRef zgas_f;
          zgas_f = z2zE3343.ztup0;
          struct zRlpCursor z1zE51;
          z1zE51 = z2zE3343.ztup1;
          struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3344;
          {
            z2zE3344 = zrlp_cursor_pop(z1zE51);
            if (have_exception) {
              KILL(zTransaction)(&z3zE3308);
              KILL(zTransaction)(&z3zE3309);
              KILL(zTransaction)(&z3zE3310);
              KILL(zTransaction)(&z3zE3311);
              goto end_block_exception_3395;
            }
          }
          struct zTransaction z3zE3312;
          CREATE(zTransaction)(&z3zE3312);
          {
            struct zRlpFieldRef zto_f;
            zto_f = z2zE3344.ztup0;
            struct zRlpCursor z1zE52;
            z1zE52 = z2zE3344.ztup1;
            struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3345;
            {
              z2zE3345 = zrlp_cursor_pop(z1zE52);
              if (have_exception) {
                KILL(zTransaction)(&z3zE3308);
                KILL(zTransaction)(&z3zE3309);
                KILL(zTransaction)(&z3zE3310);
                KILL(zTransaction)(&z3zE3311);
                KILL(zTransaction)(&z3zE3312);
                goto end_block_exception_3395;
              }
            }
            struct zTransaction z3zE3313;
            CREATE(zTransaction)(&z3zE3313);
            {
              struct zRlpFieldRef zvalue_f;
              zvalue_f = z2zE3345.ztup0;
              struct zRlpCursor z1zE53;
              z1zE53 = z2zE3345.ztup1;
              struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3346;
              {
                z2zE3346 = zrlp_cursor_pop(z1zE53);
                if (have_exception) {
                  KILL(zTransaction)(&z3zE3308);
                  KILL(zTransaction)(&z3zE3309);
                  KILL(zTransaction)(&z3zE3310);
                  KILL(zTransaction)(&z3zE3311);
                  KILL(zTransaction)(&z3zE3312);
                  KILL(zTransaction)(&z3zE3313);
                  goto end_block_exception_3395;
                }
              }
              struct zTransaction z3zE3314;
              CREATE(zTransaction)(&z3zE3314);
              {
                struct zRlpFieldRef zdata_f;
                zdata_f = z2zE3346.ztup0;
                struct zRlpCursor z1zE54;
                z1zE54 = z2zE3346.ztup1;
                struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3347;
                {
                  z2zE3347 = zrlp_cursor_pop(z1zE54);
                  if (have_exception) {
                    KILL(zTransaction)(&z3zE3308);
                    KILL(zTransaction)(&z3zE3309);
                    KILL(zTransaction)(&z3zE3310);
                    KILL(zTransaction)(&z3zE3311);
                    KILL(zTransaction)(&z3zE3312);
                    KILL(zTransaction)(&z3zE3313);
                    KILL(zTransaction)(&z3zE3314);
                    goto end_block_exception_3395;
                  }
                }
                struct zTransaction z3zE3315;
                CREATE(zTransaction)(&z3zE3315);
                {
                  struct zRlpFieldRef zal_f;
                  zal_f = z2zE3347.ztup0;
                  struct zRlpCursor z1zE55;
                  z1zE55 = z2zE3347.ztup1;
                  struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3348;
                  {
                    z2zE3348 = zrlp_cursor_pop(z1zE55);
                    if (have_exception) {
                      KILL(zTransaction)(&z3zE3308);
                      KILL(zTransaction)(&z3zE3309);
                      KILL(zTransaction)(&z3zE3310);
                      KILL(zTransaction)(&z3zE3311);
                      KILL(zTransaction)(&z3zE3312);
                      KILL(zTransaction)(&z3zE3313);
                      KILL(zTransaction)(&z3zE3314);
                      KILL(zTransaction)(&z3zE3315);
                      goto end_block_exception_3395;
                    }
                  }
                  struct zTransaction z3zE3316;
                  CREATE(zTransaction)(&z3zE3316);
                  {
                    struct zRlpFieldRef zv_f;
                    zv_f = z2zE3348.ztup0;
                    struct zRlpCursor z1zE56;
                    z1zE56 = z2zE3348.ztup1;
                    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3349;
                    {
                      z2zE3349 = zrlp_cursor_pop(z1zE56);
                      if (have_exception) {
                        KILL(zTransaction)(&z3zE3308);
                        KILL(zTransaction)(&z3zE3309);
                        KILL(zTransaction)(&z3zE3310);
                        KILL(zTransaction)(&z3zE3311);
                        KILL(zTransaction)(&z3zE3312);
                        KILL(zTransaction)(&z3zE3313);
                        KILL(zTransaction)(&z3zE3314);
                        KILL(zTransaction)(&z3zE3315);
                        KILL(zTransaction)(&z3zE3316);
                        goto end_block_exception_3395;
                      }
                    }
                    struct zTransaction z3zE3317;
                    CREATE(zTransaction)(&z3zE3317);
                    {
                      struct zRlpFieldRef zr_f;
                      zr_f = z2zE3349.ztup0;
                      struct zRlpCursor z1zE57;
                      z1zE57 = z2zE3349.ztup1;
                      struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3350;
                      {
                        z2zE3350 = zrlp_cursor_pop(z1zE57);
                        if (have_exception) {
                          KILL(zTransaction)(&z3zE3308);
                          KILL(zTransaction)(&z3zE3309);
                          KILL(zTransaction)(&z3zE3310);
                          KILL(zTransaction)(&z3zE3311);
                          KILL(zTransaction)(&z3zE3312);
                          KILL(zTransaction)(&z3zE3313);
                          KILL(zTransaction)(&z3zE3314);
                          KILL(zTransaction)(&z3zE3315);
                          KILL(zTransaction)(&z3zE3316);
                          KILL(zTransaction)(&z3zE3317);
                          goto end_block_exception_3395;
                        }
                      }
                      struct zTransaction z3zE3318;
                      CREATE(zTransaction)(&z3zE3318);
                      {
                        struct zRlpFieldRef zs_f;
                        zs_f = z2zE3350.ztup0;
                        struct zRlpCursor z1zE58;
                        z1zE58 = z2zE3350.ztup1;
                        unit z3zE3319;
                        {
                          z3zE3319 = zrlp_cursor_expect_end(z1zE58);
                          if (have_exception) {
                            KILL(zTransaction)(&z3zE3308);
                            KILL(zTransaction)(&z3zE3309);
                            KILL(zTransaction)(&z3zE3310);
                            KILL(zTransaction)(&z3zE3311);
                            KILL(zTransaction)(&z3zE3312);
                            KILL(zTransaction)(&z3zE3313);
                            KILL(zTransaction)(&z3zE3314);
                            KILL(zTransaction)(&z3zE3315);
                            KILL(zTransaction)(&z3zE3316);
                            KILL(zTransaction)(&z3zE3317);
                            KILL(zTransaction)(&z3zE3318);
                            goto end_block_exception_3395;
                          }
                        }
                        sail_u256 zv;
                        {
                          zv = zrlp_ref_word(zv_f);
                          if (have_exception) {
                            KILL(zTransaction)(&z3zE3308);
                            KILL(zTransaction)(&z3zE3309);
                            KILL(zTransaction)(&z3zE3310);
                            KILL(zTransaction)(&z3zE3311);
                            KILL(zTransaction)(&z3zE3312);
                            KILL(zTransaction)(&z3zE3313);
                            KILL(zTransaction)(&z3zE3314);
                            KILL(zTransaction)(&z3zE3315);
                            KILL(zTransaction)(&z3zE3316);
                            KILL(zTransaction)(&z3zE3317);
                            KILL(zTransaction)(&z3zE3318);
                            goto end_block_exception_3395;
                          }
                        }
                        sail_u256 zgp;
                        {
                          zgp = zrlp_ref_uint_word(zgp_f);
                          if (have_exception) {
                            KILL(zTransaction)(&z3zE3308);
                            KILL(zTransaction)(&z3zE3309);
                            KILL(zTransaction)(&z3zE3310);
                            KILL(zTransaction)(&z3zE3311);
                            KILL(zTransaction)(&z3zE3312);
                            KILL(zTransaction)(&z3zE3313);
                            KILL(zTransaction)(&z3zE3314);
                            KILL(zTransaction)(&z3zE3315);
                            KILL(zTransaction)(&z3zE3316);
                            KILL(zTransaction)(&z3zE3317);
                            KILL(zTransaction)(&z3zE3318);
                            goto end_block_exception_3395;
                          }
                        }
                        struct ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9 z2zE3351;
                        CREATE(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3351);
                        {
                          zdecode_access_list(&z2zE3351, zal_f);
                          if (have_exception) {
                            KILL(zTransaction)(&z3zE3308);
                            KILL(zTransaction)(&z3zE3309);
                            KILL(zTransaction)(&z3zE3310);
                            KILL(zTransaction)(&z3zE3311);
                            KILL(zTransaction)(&z3zE3312);
                            KILL(zTransaction)(&z3zE3313);
                            KILL(zTransaction)(&z3zE3314);
                            KILL(zTransaction)(&z3zE3315);
                            KILL(zTransaction)(&z3zE3316);
                            KILL(zTransaction)(&z3zE3317);
                            KILL(zTransaction)(&z3zE3318);
                            KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3351);
                            goto end_block_exception_3395;
                          }
                        }
                        struct zTransaction z3zE3320;
                        CREATE(zTransaction)(&z3zE3320);
                        {
                          zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 zal_addrs;
                          CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                          COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs, z2zE3351.ztup0);
                          zz5listz8z5structz0zzStorageKeyz9 zal_slots;
                          CREATE(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                          COPY(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots, z2zE3351.ztup1);
                          uint64_t zal_addr_count;
                          zal_addr_count = z2zE3351.ztup2;
                          uint64_t zal_slot_count;
                          zal_slot_count = z2zE3351.ztup3;
                          sail_u256 z2zE3352;
                          {
                            z2zE3352 = zrlp_ref_uint_word(znonce_f);
                            if (have_exception) {
                              KILL(zTransaction)(&z3zE3308);
                              KILL(zTransaction)(&z3zE3309);
                              KILL(zTransaction)(&z3zE3310);
                              KILL(zTransaction)(&z3zE3311);
                              KILL(zTransaction)(&z3zE3312);
                              KILL(zTransaction)(&z3zE3313);
                              KILL(zTransaction)(&z3zE3314);
                              KILL(zTransaction)(&z3zE3315);
                              KILL(zTransaction)(&z3zE3316);
                              KILL(zTransaction)(&z3zE3317);
                              KILL(zTransaction)(&z3zE3318);
                              KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3351);
                              KILL(zTransaction)(&z3zE3320);
                              KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                              KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                              goto end_block_exception_3395;
                            }
                          }
                          uint64_t z2zE3353;
                          {
                            z2zE3353 = zrlp_ref_uint64(zchain_f);
                            if (have_exception) {
                              KILL(zTransaction)(&z3zE3308);
                              KILL(zTransaction)(&z3zE3309);
                              KILL(zTransaction)(&z3zE3310);
                              KILL(zTransaction)(&z3zE3311);
                              KILL(zTransaction)(&z3zE3312);
                              KILL(zTransaction)(&z3zE3313);
                              KILL(zTransaction)(&z3zE3314);
                              KILL(zTransaction)(&z3zE3315);
                              KILL(zTransaction)(&z3zE3316);
                              KILL(zTransaction)(&z3zE3317);
                              KILL(zTransaction)(&z3zE3318);
                              KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3351);
                              KILL(zTransaction)(&z3zE3320);
                              KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                              KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                              goto end_block_exception_3395;
                            }
                          }
                          uint64_t z2zE3354;
                          {
                            z2zE3354 = zrlp_ref_gas(zgas_f, zfork);
                            if (have_exception) {
                              KILL(zTransaction)(&z3zE3308);
                              KILL(zTransaction)(&z3zE3309);
                              KILL(zTransaction)(&z3zE3310);
                              KILL(zTransaction)(&z3zE3311);
                              KILL(zTransaction)(&z3zE3312);
                              KILL(zTransaction)(&z3zE3313);
                              KILL(zTransaction)(&z3zE3314);
                              KILL(zTransaction)(&z3zE3315);
                              KILL(zTransaction)(&z3zE3316);
                              KILL(zTransaction)(&z3zE3317);
                              KILL(zTransaction)(&z3zE3318);
                              KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3351);
                              KILL(zTransaction)(&z3zE3320);
                              KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                              KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                              goto end_block_exception_3395;
                            }
                          }
                          bool z2zE3356;
                          {
                            uint64_t z2zE3355;
                            z2zE3355 = zto_f.zcontent_len;
                            z2zE3356 = (z2zE3355 == UINT64_C(0));
                          }
                          sail_fixed_bytes_20 z2zE3358;
                          {
                            sail_u256 z2zE3357;
                            {
                              z2zE3357 = zrlp_ref_word(zto_f);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE3308);
                                KILL(zTransaction)(&z3zE3309);
                                KILL(zTransaction)(&z3zE3310);
                                KILL(zTransaction)(&z3zE3311);
                                KILL(zTransaction)(&z3zE3312);
                                KILL(zTransaction)(&z3zE3313);
                                KILL(zTransaction)(&z3zE3314);
                                KILL(zTransaction)(&z3zE3315);
                                KILL(zTransaction)(&z3zE3316);
                                KILL(zTransaction)(&z3zE3317);
                                KILL(zTransaction)(&z3zE3318);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3351);
                                KILL(zTransaction)(&z3zE3320);
                                KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                goto end_block_exception_3395;
                              }
                            }
                            z2zE3358 = zword_to_address(z2zE3357);
                          }
                          sail_u256 z2zE3359;
                          {
                            z2zE3359 = zrlp_ref_uint_word(zvalue_f);
                            if (have_exception) {
                              KILL(zTransaction)(&z3zE3308);
                              KILL(zTransaction)(&z3zE3309);
                              KILL(zTransaction)(&z3zE3310);
                              KILL(zTransaction)(&z3zE3311);
                              KILL(zTransaction)(&z3zE3312);
                              KILL(zTransaction)(&z3zE3313);
                              KILL(zTransaction)(&z3zE3314);
                              KILL(zTransaction)(&z3zE3315);
                              KILL(zTransaction)(&z3zE3316);
                              KILL(zTransaction)(&z3zE3317);
                              KILL(zTransaction)(&z3zE3318);
                              KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3351);
                              KILL(zTransaction)(&z3zE3320);
                              KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                              KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                              goto end_block_exception_3395;
                            }
                          }
                          struct zByteSliceFields z2zE3360;
                          {
                            z2zE3360 = ztx_input_span(zdata_f);
                            if (have_exception) {
                              KILL(zTransaction)(&z3zE3308);
                              KILL(zTransaction)(&z3zE3309);
                              KILL(zTransaction)(&z3zE3310);
                              KILL(zTransaction)(&z3zE3311);
                              KILL(zTransaction)(&z3zE3312);
                              KILL(zTransaction)(&z3zE3313);
                              KILL(zTransaction)(&z3zE3314);
                              KILL(zTransaction)(&z3zE3315);
                              KILL(zTransaction)(&z3zE3316);
                              KILL(zTransaction)(&z3zE3317);
                              KILL(zTransaction)(&z3zE3318);
                              KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3351);
                              KILL(zTransaction)(&z3zE3320);
                              KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                              KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                              goto end_block_exception_3395;
                            }
                          }
                          sail_fixed_bytes_32 z2zE3362;
                          {
                            struct zByteSliceFields z2zE3361;
                            {
                              z2zE3361 = ztx_sig_span(zchain_f, zv_f);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE3308);
                                KILL(zTransaction)(&z3zE3309);
                                KILL(zTransaction)(&z3zE3310);
                                KILL(zTransaction)(&z3zE3311);
                                KILL(zTransaction)(&z3zE3312);
                                KILL(zTransaction)(&z3zE3313);
                                KILL(zTransaction)(&z3zE3314);
                                KILL(zTransaction)(&z3zE3315);
                                KILL(zTransaction)(&z3zE3316);
                                KILL(zTransaction)(&z3zE3317);
                                KILL(zTransaction)(&z3zE3318);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3351);
                                KILL(zTransaction)(&z3zE3320);
                                KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                goto end_block_exception_3395;
                              }
                            }
                            {
                              z2zE3362 = ztx_signing_hash(zAccessListTx, z2zE3361, zv);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE3308);
                                KILL(zTransaction)(&z3zE3309);
                                KILL(zTransaction)(&z3zE3310);
                                KILL(zTransaction)(&z3zE3311);
                                KILL(zTransaction)(&z3zE3312);
                                KILL(zTransaction)(&z3zE3313);
                                KILL(zTransaction)(&z3zE3314);
                                KILL(zTransaction)(&z3zE3315);
                                KILL(zTransaction)(&z3zE3316);
                                KILL(zTransaction)(&z3zE3317);
                                KILL(zTransaction)(&z3zE3318);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3351);
                                KILL(zTransaction)(&z3zE3320);
                                KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                goto end_block_exception_3395;
                              }
                            }
                          }
                          sail_u256 z2zE3363;
                          {
                            z2zE3363 = zrlp_ref_uint_word(zr_f);
                            if (have_exception) {
                              KILL(zTransaction)(&z3zE3308);
                              KILL(zTransaction)(&z3zE3309);
                              KILL(zTransaction)(&z3zE3310);
                              KILL(zTransaction)(&z3zE3311);
                              KILL(zTransaction)(&z3zE3312);
                              KILL(zTransaction)(&z3zE3313);
                              KILL(zTransaction)(&z3zE3314);
                              KILL(zTransaction)(&z3zE3315);
                              KILL(zTransaction)(&z3zE3316);
                              KILL(zTransaction)(&z3zE3317);
                              KILL(zTransaction)(&z3zE3318);
                              KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3351);
                              KILL(zTransaction)(&z3zE3320);
                              KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                              KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                              goto end_block_exception_3395;
                            }
                          }
                          sail_u256 z2zE3364;
                          {
                            z2zE3364 = zrlp_ref_uint_word(zs_f);
                            if (have_exception) {
                              KILL(zTransaction)(&z3zE3308);
                              KILL(zTransaction)(&z3zE3309);
                              KILL(zTransaction)(&z3zE3310);
                              KILL(zTransaction)(&z3zE3311);
                              KILL(zTransaction)(&z3zE3312);
                              KILL(zTransaction)(&z3zE3313);
                              KILL(zTransaction)(&z3zE3314);
                              KILL(zTransaction)(&z3zE3315);
                              KILL(zTransaction)(&z3zE3316);
                              KILL(zTransaction)(&z3zE3317);
                              KILL(zTransaction)(&z3zE3318);
                              KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3351);
                              KILL(zTransaction)(&z3zE3320);
                              KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                              KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                              goto end_block_exception_3395;
                            }
                          }
                          z3zE3320.zaccess_list_address_count = zal_addr_count;
                          COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE3320)->zaccess_list_addresses), zal_addrs);
                          z3zE3320.zaccess_list_slot_count = zal_slot_count;
                          COPY(zz5listz8z5structz0zzStorageKeyz9)(&((&z3zE3320)->zaccess_list_slots), zal_slots);
                          z3zE3320.zauthorizzation_count = UINT64_C(0);
                          zz5listz8z5structz0zzAuthorizzzzationz9 z3zE3322;
                          CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&z3zE3322);
                          COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&((&z3zE3320)->zauthorizzations), z3zE3322);
                          KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z3zE3322);
                          z3zE3320.zblob_hashes = zEMPTY_BLOB_HASHES;
                          z3zE3320.zchain_id = z2zE3353;
                          z3zE3320.zgas_limit = z2zE3354;
                          z3zE3320.zinput_src = z2zE3360;
                          z3zE3320.zis_create = z2zE3356;
                          z3zE3320.zmax_blob_fee = zZERO_WORD;
                          z3zE3320.zmax_fee = zgp;
                          z3zE3320.zmax_priority_fee = zgp;
                          z3zE3320.znonce = z2zE3352;
                          z3zE3320.zpubkey = zpubkey;
                          z3zE3320.zraw = ztx;
                          z3zE3320.zrecipient = z2zE3358;
                          z3zE3320.zsender = zsender;
                          z3zE3320.zsig_r = z2zE3363;
                          z3zE3320.zsig_s = z2zE3364;
                          z3zE3320.zsig_v = zv;
                          z3zE3320.zsigning_hash = z2zE3362;
                          z3zE3320.ztx_type = zAccessListTx;
                          z3zE3320.zvalue = z2zE3359;
                          KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                          KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                          goto finish_match_3392;
                        }
                      case_3393: ;
                        sail_match_failure("decode_access_list_tx");
                      finish_match_3392: ;
                        COPY(zTransaction)(&z3zE3318, z3zE3320);
                        KILL(zTransaction)(&z3zE3320);
                        KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3351);
                        goto finish_match_3390;
                      }
                    case_3391: ;
                      sail_match_failure("decode_access_list_tx");
                    finish_match_3390: ;
                      COPY(zTransaction)(&z3zE3317, z3zE3318);
                      KILL(zTransaction)(&z3zE3318);
                      goto finish_match_3388;
                    }
                  case_3389: ;
                    sail_match_failure("decode_access_list_tx");
                  finish_match_3388: ;
                    COPY(zTransaction)(&z3zE3316, z3zE3317);
                    KILL(zTransaction)(&z3zE3317);
                    goto finish_match_3386;
                  }
                case_3387: ;
                  sail_match_failure("decode_access_list_tx");
                finish_match_3386: ;
                  COPY(zTransaction)(&z3zE3315, z3zE3316);
                  KILL(zTransaction)(&z3zE3316);
                  goto finish_match_3384;
                }
              case_3385: ;
                sail_match_failure("decode_access_list_tx");
              finish_match_3384: ;
                COPY(zTransaction)(&z3zE3314, z3zE3315);
                KILL(zTransaction)(&z3zE3315);
                goto finish_match_3382;
              }
            case_3383: ;
              sail_match_failure("decode_access_list_tx");
            finish_match_3382: ;
              COPY(zTransaction)(&z3zE3313, z3zE3314);
              KILL(zTransaction)(&z3zE3314);
              goto finish_match_3380;
            }
          case_3381: ;
            sail_match_failure("decode_access_list_tx");
          finish_match_3380: ;
            COPY(zTransaction)(&z3zE3312, z3zE3313);
            KILL(zTransaction)(&z3zE3313);
            goto finish_match_3378;
          }
        case_3379: ;
          sail_match_failure("decode_access_list_tx");
        finish_match_3378: ;
          COPY(zTransaction)(&z3zE3311, z3zE3312);
          KILL(zTransaction)(&z3zE3312);
          goto finish_match_3376;
        }
      case_3377: ;
        sail_match_failure("decode_access_list_tx");
      finish_match_3376: ;
        COPY(zTransaction)(&z3zE3310, z3zE3311);
        KILL(zTransaction)(&z3zE3311);
        goto finish_match_3374;
      }
    case_3375: ;
      sail_match_failure("decode_access_list_tx");
    finish_match_3374: ;
      COPY(zTransaction)(&z3zE3309, z3zE3310);
      KILL(zTransaction)(&z3zE3310);
      goto finish_match_3372;
    }
  case_3373: ;
    sail_match_failure("decode_access_list_tx");
  finish_match_3372: ;
    COPY(zTransaction)(&z3zE3308, z3zE3309);
    KILL(zTransaction)(&z3zE3309);
    goto finish_match_3370;
  }
case_3371: ;
  sail_match_failure("decode_access_list_tx");
finish_match_3370: ;
  COPY(zTransaction)((*(&z8zE200)), z3zE3308);
  KILL(zTransaction)(&z3zE3308);
end_function_3394: ;
  goto end_function_4065;
end_block_exception_3395: ;
  goto end_function_4065;
end_function_4065: ;
}

void zdecode_fee_market_tx(struct zTransaction *z8zE201, struct zByteSliceFields ztx, struct zByteSliceFields zpubkey, enum zFork zfork, sail_fixed_bytes_20 zsender, struct zRlpCursor zfields)
{
  struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3312;
  {
    z2zE3312 = zrlp_cursor_pop(zfields);
    if (have_exception) {  goto end_block_exception_3369;  }
  }
  struct zTransaction z3zE3279;
  CREATE(zTransaction)(&z3zE3279);
  {
    struct zRlpFieldRef zchain_f;
    zchain_f = z2zE3312.ztup0;
    struct zRlpCursor z1zE36;
    z1zE36 = z2zE3312.ztup1;
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3313;
    {
      z2zE3313 = zrlp_cursor_pop(z1zE36);
      if (have_exception) {
        KILL(zTransaction)(&z3zE3279);
        goto end_block_exception_3369;
      }
    }
    struct zTransaction z3zE3280;
    CREATE(zTransaction)(&z3zE3280);
    {
      struct zRlpFieldRef znonce_f;
      znonce_f = z2zE3313.ztup0;
      struct zRlpCursor z1zE37;
      z1zE37 = z2zE3313.ztup1;
      struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3314;
      {
        z2zE3314 = zrlp_cursor_pop(z1zE37);
        if (have_exception) {
          KILL(zTransaction)(&z3zE3279);
          KILL(zTransaction)(&z3zE3280);
          goto end_block_exception_3369;
        }
      }
      struct zTransaction z3zE3281;
      CREATE(zTransaction)(&z3zE3281);
      {
        struct zRlpFieldRef zmp_f;
        zmp_f = z2zE3314.ztup0;
        struct zRlpCursor z1zE38;
        z1zE38 = z2zE3314.ztup1;
        struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3315;
        {
          z2zE3315 = zrlp_cursor_pop(z1zE38);
          if (have_exception) {
            KILL(zTransaction)(&z3zE3279);
            KILL(zTransaction)(&z3zE3280);
            KILL(zTransaction)(&z3zE3281);
            goto end_block_exception_3369;
          }
        }
        struct zTransaction z3zE3282;
        CREATE(zTransaction)(&z3zE3282);
        {
          struct zRlpFieldRef zmf_f;
          zmf_f = z2zE3315.ztup0;
          struct zRlpCursor z1zE39;
          z1zE39 = z2zE3315.ztup1;
          struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3316;
          {
            z2zE3316 = zrlp_cursor_pop(z1zE39);
            if (have_exception) {
              KILL(zTransaction)(&z3zE3279);
              KILL(zTransaction)(&z3zE3280);
              KILL(zTransaction)(&z3zE3281);
              KILL(zTransaction)(&z3zE3282);
              goto end_block_exception_3369;
            }
          }
          struct zTransaction z3zE3283;
          CREATE(zTransaction)(&z3zE3283);
          {
            struct zRlpFieldRef zgas_f;
            zgas_f = z2zE3316.ztup0;
            struct zRlpCursor z1zE40;
            z1zE40 = z2zE3316.ztup1;
            struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3317;
            {
              z2zE3317 = zrlp_cursor_pop(z1zE40);
              if (have_exception) {
                KILL(zTransaction)(&z3zE3279);
                KILL(zTransaction)(&z3zE3280);
                KILL(zTransaction)(&z3zE3281);
                KILL(zTransaction)(&z3zE3282);
                KILL(zTransaction)(&z3zE3283);
                goto end_block_exception_3369;
              }
            }
            struct zTransaction z3zE3284;
            CREATE(zTransaction)(&z3zE3284);
            {
              struct zRlpFieldRef zto_f;
              zto_f = z2zE3317.ztup0;
              struct zRlpCursor z1zE41;
              z1zE41 = z2zE3317.ztup1;
              struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3318;
              {
                z2zE3318 = zrlp_cursor_pop(z1zE41);
                if (have_exception) {
                  KILL(zTransaction)(&z3zE3279);
                  KILL(zTransaction)(&z3zE3280);
                  KILL(zTransaction)(&z3zE3281);
                  KILL(zTransaction)(&z3zE3282);
                  KILL(zTransaction)(&z3zE3283);
                  KILL(zTransaction)(&z3zE3284);
                  goto end_block_exception_3369;
                }
              }
              struct zTransaction z3zE3285;
              CREATE(zTransaction)(&z3zE3285);
              {
                struct zRlpFieldRef zvalue_f;
                zvalue_f = z2zE3318.ztup0;
                struct zRlpCursor z1zE42;
                z1zE42 = z2zE3318.ztup1;
                struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3319;
                {
                  z2zE3319 = zrlp_cursor_pop(z1zE42);
                  if (have_exception) {
                    KILL(zTransaction)(&z3zE3279);
                    KILL(zTransaction)(&z3zE3280);
                    KILL(zTransaction)(&z3zE3281);
                    KILL(zTransaction)(&z3zE3282);
                    KILL(zTransaction)(&z3zE3283);
                    KILL(zTransaction)(&z3zE3284);
                    KILL(zTransaction)(&z3zE3285);
                    goto end_block_exception_3369;
                  }
                }
                struct zTransaction z3zE3286;
                CREATE(zTransaction)(&z3zE3286);
                {
                  struct zRlpFieldRef zdata_f;
                  zdata_f = z2zE3319.ztup0;
                  struct zRlpCursor z1zE43;
                  z1zE43 = z2zE3319.ztup1;
                  struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3320;
                  {
                    z2zE3320 = zrlp_cursor_pop(z1zE43);
                    if (have_exception) {
                      KILL(zTransaction)(&z3zE3279);
                      KILL(zTransaction)(&z3zE3280);
                      KILL(zTransaction)(&z3zE3281);
                      KILL(zTransaction)(&z3zE3282);
                      KILL(zTransaction)(&z3zE3283);
                      KILL(zTransaction)(&z3zE3284);
                      KILL(zTransaction)(&z3zE3285);
                      KILL(zTransaction)(&z3zE3286);
                      goto end_block_exception_3369;
                    }
                  }
                  struct zTransaction z3zE3287;
                  CREATE(zTransaction)(&z3zE3287);
                  {
                    struct zRlpFieldRef zal_f;
                    zal_f = z2zE3320.ztup0;
                    struct zRlpCursor z1zE44;
                    z1zE44 = z2zE3320.ztup1;
                    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3321;
                    {
                      z2zE3321 = zrlp_cursor_pop(z1zE44);
                      if (have_exception) {
                        KILL(zTransaction)(&z3zE3279);
                        KILL(zTransaction)(&z3zE3280);
                        KILL(zTransaction)(&z3zE3281);
                        KILL(zTransaction)(&z3zE3282);
                        KILL(zTransaction)(&z3zE3283);
                        KILL(zTransaction)(&z3zE3284);
                        KILL(zTransaction)(&z3zE3285);
                        KILL(zTransaction)(&z3zE3286);
                        KILL(zTransaction)(&z3zE3287);
                        goto end_block_exception_3369;
                      }
                    }
                    struct zTransaction z3zE3288;
                    CREATE(zTransaction)(&z3zE3288);
                    {
                      struct zRlpFieldRef zv_f;
                      zv_f = z2zE3321.ztup0;
                      struct zRlpCursor z1zE45;
                      z1zE45 = z2zE3321.ztup1;
                      struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3322;
                      {
                        z2zE3322 = zrlp_cursor_pop(z1zE45);
                        if (have_exception) {
                          KILL(zTransaction)(&z3zE3279);
                          KILL(zTransaction)(&z3zE3280);
                          KILL(zTransaction)(&z3zE3281);
                          KILL(zTransaction)(&z3zE3282);
                          KILL(zTransaction)(&z3zE3283);
                          KILL(zTransaction)(&z3zE3284);
                          KILL(zTransaction)(&z3zE3285);
                          KILL(zTransaction)(&z3zE3286);
                          KILL(zTransaction)(&z3zE3287);
                          KILL(zTransaction)(&z3zE3288);
                          goto end_block_exception_3369;
                        }
                      }
                      struct zTransaction z3zE3289;
                      CREATE(zTransaction)(&z3zE3289);
                      {
                        struct zRlpFieldRef zr_f;
                        zr_f = z2zE3322.ztup0;
                        struct zRlpCursor z1zE46;
                        z1zE46 = z2zE3322.ztup1;
                        struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3323;
                        {
                          z2zE3323 = zrlp_cursor_pop(z1zE46);
                          if (have_exception) {
                            KILL(zTransaction)(&z3zE3279);
                            KILL(zTransaction)(&z3zE3280);
                            KILL(zTransaction)(&z3zE3281);
                            KILL(zTransaction)(&z3zE3282);
                            KILL(zTransaction)(&z3zE3283);
                            KILL(zTransaction)(&z3zE3284);
                            KILL(zTransaction)(&z3zE3285);
                            KILL(zTransaction)(&z3zE3286);
                            KILL(zTransaction)(&z3zE3287);
                            KILL(zTransaction)(&z3zE3288);
                            KILL(zTransaction)(&z3zE3289);
                            goto end_block_exception_3369;
                          }
                        }
                        struct zTransaction z3zE3290;
                        CREATE(zTransaction)(&z3zE3290);
                        {
                          struct zRlpFieldRef zs_f;
                          zs_f = z2zE3323.ztup0;
                          struct zRlpCursor z1zE47;
                          z1zE47 = z2zE3323.ztup1;
                          unit z3zE3291;
                          {
                            z3zE3291 = zrlp_cursor_expect_end(z1zE47);
                            if (have_exception) {
                              KILL(zTransaction)(&z3zE3279);
                              KILL(zTransaction)(&z3zE3280);
                              KILL(zTransaction)(&z3zE3281);
                              KILL(zTransaction)(&z3zE3282);
                              KILL(zTransaction)(&z3zE3283);
                              KILL(zTransaction)(&z3zE3284);
                              KILL(zTransaction)(&z3zE3285);
                              KILL(zTransaction)(&z3zE3286);
                              KILL(zTransaction)(&z3zE3287);
                              KILL(zTransaction)(&z3zE3288);
                              KILL(zTransaction)(&z3zE3289);
                              KILL(zTransaction)(&z3zE3290);
                              goto end_block_exception_3369;
                            }
                          }
                          sail_u256 zv;
                          {
                            zv = zrlp_ref_word(zv_f);
                            if (have_exception) {
                              KILL(zTransaction)(&z3zE3279);
                              KILL(zTransaction)(&z3zE3280);
                              KILL(zTransaction)(&z3zE3281);
                              KILL(zTransaction)(&z3zE3282);
                              KILL(zTransaction)(&z3zE3283);
                              KILL(zTransaction)(&z3zE3284);
                              KILL(zTransaction)(&z3zE3285);
                              KILL(zTransaction)(&z3zE3286);
                              KILL(zTransaction)(&z3zE3287);
                              KILL(zTransaction)(&z3zE3288);
                              KILL(zTransaction)(&z3zE3289);
                              KILL(zTransaction)(&z3zE3290);
                              goto end_block_exception_3369;
                            }
                          }
                          struct ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9 z2zE3324;
                          CREATE(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3324);
                          {
                            zdecode_access_list(&z2zE3324, zal_f);
                            if (have_exception) {
                              KILL(zTransaction)(&z3zE3279);
                              KILL(zTransaction)(&z3zE3280);
                              KILL(zTransaction)(&z3zE3281);
                              KILL(zTransaction)(&z3zE3282);
                              KILL(zTransaction)(&z3zE3283);
                              KILL(zTransaction)(&z3zE3284);
                              KILL(zTransaction)(&z3zE3285);
                              KILL(zTransaction)(&z3zE3286);
                              KILL(zTransaction)(&z3zE3287);
                              KILL(zTransaction)(&z3zE3288);
                              KILL(zTransaction)(&z3zE3289);
                              KILL(zTransaction)(&z3zE3290);
                              KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3324);
                              goto end_block_exception_3369;
                            }
                          }
                          struct zTransaction z3zE3292;
                          CREATE(zTransaction)(&z3zE3292);
                          {
                            zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 zal_addrs;
                            CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                            COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs, z2zE3324.ztup0);
                            zz5listz8z5structz0zzStorageKeyz9 zal_slots;
                            CREATE(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                            COPY(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots, z2zE3324.ztup1);
                            uint64_t zal_addr_count;
                            zal_addr_count = z2zE3324.ztup2;
                            uint64_t zal_slot_count;
                            zal_slot_count = z2zE3324.ztup3;
                            sail_u256 z2zE3325;
                            {
                              z2zE3325 = zrlp_ref_uint_word(znonce_f);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE3279);
                                KILL(zTransaction)(&z3zE3280);
                                KILL(zTransaction)(&z3zE3281);
                                KILL(zTransaction)(&z3zE3282);
                                KILL(zTransaction)(&z3zE3283);
                                KILL(zTransaction)(&z3zE3284);
                                KILL(zTransaction)(&z3zE3285);
                                KILL(zTransaction)(&z3zE3286);
                                KILL(zTransaction)(&z3zE3287);
                                KILL(zTransaction)(&z3zE3288);
                                KILL(zTransaction)(&z3zE3289);
                                KILL(zTransaction)(&z3zE3290);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3324);
                                KILL(zTransaction)(&z3zE3292);
                                KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                goto end_block_exception_3369;
                              }
                            }
                            uint64_t z2zE3326;
                            {
                              z2zE3326 = zrlp_ref_uint64(zchain_f);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE3279);
                                KILL(zTransaction)(&z3zE3280);
                                KILL(zTransaction)(&z3zE3281);
                                KILL(zTransaction)(&z3zE3282);
                                KILL(zTransaction)(&z3zE3283);
                                KILL(zTransaction)(&z3zE3284);
                                KILL(zTransaction)(&z3zE3285);
                                KILL(zTransaction)(&z3zE3286);
                                KILL(zTransaction)(&z3zE3287);
                                KILL(zTransaction)(&z3zE3288);
                                KILL(zTransaction)(&z3zE3289);
                                KILL(zTransaction)(&z3zE3290);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3324);
                                KILL(zTransaction)(&z3zE3292);
                                KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                goto end_block_exception_3369;
                              }
                            }
                            uint64_t z2zE3327;
                            {
                              z2zE3327 = zrlp_ref_gas(zgas_f, zfork);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE3279);
                                KILL(zTransaction)(&z3zE3280);
                                KILL(zTransaction)(&z3zE3281);
                                KILL(zTransaction)(&z3zE3282);
                                KILL(zTransaction)(&z3zE3283);
                                KILL(zTransaction)(&z3zE3284);
                                KILL(zTransaction)(&z3zE3285);
                                KILL(zTransaction)(&z3zE3286);
                                KILL(zTransaction)(&z3zE3287);
                                KILL(zTransaction)(&z3zE3288);
                                KILL(zTransaction)(&z3zE3289);
                                KILL(zTransaction)(&z3zE3290);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3324);
                                KILL(zTransaction)(&z3zE3292);
                                KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                goto end_block_exception_3369;
                              }
                            }
                            bool z2zE3329;
                            {
                              uint64_t z2zE3328;
                              z2zE3328 = zto_f.zcontent_len;
                              z2zE3329 = (z2zE3328 == UINT64_C(0));
                            }
                            sail_fixed_bytes_20 z2zE3331;
                            {
                              sail_u256 z2zE3330;
                              {
                                z2zE3330 = zrlp_ref_word(zto_f);
                                if (have_exception) {
                                  KILL(zTransaction)(&z3zE3279);
                                  KILL(zTransaction)(&z3zE3280);
                                  KILL(zTransaction)(&z3zE3281);
                                  KILL(zTransaction)(&z3zE3282);
                                  KILL(zTransaction)(&z3zE3283);
                                  KILL(zTransaction)(&z3zE3284);
                                  KILL(zTransaction)(&z3zE3285);
                                  KILL(zTransaction)(&z3zE3286);
                                  KILL(zTransaction)(&z3zE3287);
                                  KILL(zTransaction)(&z3zE3288);
                                  KILL(zTransaction)(&z3zE3289);
                                  KILL(zTransaction)(&z3zE3290);
                                  KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3324);
                                  KILL(zTransaction)(&z3zE3292);
                                  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                  KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                  goto end_block_exception_3369;
                                }
                              }
                              z2zE3331 = zword_to_address(z2zE3330);
                            }
                            sail_u256 z2zE3332;
                            {
                              z2zE3332 = zrlp_ref_uint_word(zvalue_f);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE3279);
                                KILL(zTransaction)(&z3zE3280);
                                KILL(zTransaction)(&z3zE3281);
                                KILL(zTransaction)(&z3zE3282);
                                KILL(zTransaction)(&z3zE3283);
                                KILL(zTransaction)(&z3zE3284);
                                KILL(zTransaction)(&z3zE3285);
                                KILL(zTransaction)(&z3zE3286);
                                KILL(zTransaction)(&z3zE3287);
                                KILL(zTransaction)(&z3zE3288);
                                KILL(zTransaction)(&z3zE3289);
                                KILL(zTransaction)(&z3zE3290);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3324);
                                KILL(zTransaction)(&z3zE3292);
                                KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                goto end_block_exception_3369;
                              }
                            }
                            struct zByteSliceFields z2zE3333;
                            {
                              z2zE3333 = ztx_input_span(zdata_f);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE3279);
                                KILL(zTransaction)(&z3zE3280);
                                KILL(zTransaction)(&z3zE3281);
                                KILL(zTransaction)(&z3zE3282);
                                KILL(zTransaction)(&z3zE3283);
                                KILL(zTransaction)(&z3zE3284);
                                KILL(zTransaction)(&z3zE3285);
                                KILL(zTransaction)(&z3zE3286);
                                KILL(zTransaction)(&z3zE3287);
                                KILL(zTransaction)(&z3zE3288);
                                KILL(zTransaction)(&z3zE3289);
                                KILL(zTransaction)(&z3zE3290);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3324);
                                KILL(zTransaction)(&z3zE3292);
                                KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                goto end_block_exception_3369;
                              }
                            }
                            sail_u256 z2zE3334;
                            {
                              z2zE3334 = zrlp_ref_uint_word(zmf_f);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE3279);
                                KILL(zTransaction)(&z3zE3280);
                                KILL(zTransaction)(&z3zE3281);
                                KILL(zTransaction)(&z3zE3282);
                                KILL(zTransaction)(&z3zE3283);
                                KILL(zTransaction)(&z3zE3284);
                                KILL(zTransaction)(&z3zE3285);
                                KILL(zTransaction)(&z3zE3286);
                                KILL(zTransaction)(&z3zE3287);
                                KILL(zTransaction)(&z3zE3288);
                                KILL(zTransaction)(&z3zE3289);
                                KILL(zTransaction)(&z3zE3290);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3324);
                                KILL(zTransaction)(&z3zE3292);
                                KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                goto end_block_exception_3369;
                              }
                            }
                            sail_u256 z2zE3335;
                            {
                              z2zE3335 = zrlp_ref_uint_word(zmp_f);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE3279);
                                KILL(zTransaction)(&z3zE3280);
                                KILL(zTransaction)(&z3zE3281);
                                KILL(zTransaction)(&z3zE3282);
                                KILL(zTransaction)(&z3zE3283);
                                KILL(zTransaction)(&z3zE3284);
                                KILL(zTransaction)(&z3zE3285);
                                KILL(zTransaction)(&z3zE3286);
                                KILL(zTransaction)(&z3zE3287);
                                KILL(zTransaction)(&z3zE3288);
                                KILL(zTransaction)(&z3zE3289);
                                KILL(zTransaction)(&z3zE3290);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3324);
                                KILL(zTransaction)(&z3zE3292);
                                KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                goto end_block_exception_3369;
                              }
                            }
                            sail_fixed_bytes_32 z2zE3337;
                            {
                              struct zByteSliceFields z2zE3336;
                              {
                                z2zE3336 = ztx_sig_span(zchain_f, zv_f);
                                if (have_exception) {
                                  KILL(zTransaction)(&z3zE3279);
                                  KILL(zTransaction)(&z3zE3280);
                                  KILL(zTransaction)(&z3zE3281);
                                  KILL(zTransaction)(&z3zE3282);
                                  KILL(zTransaction)(&z3zE3283);
                                  KILL(zTransaction)(&z3zE3284);
                                  KILL(zTransaction)(&z3zE3285);
                                  KILL(zTransaction)(&z3zE3286);
                                  KILL(zTransaction)(&z3zE3287);
                                  KILL(zTransaction)(&z3zE3288);
                                  KILL(zTransaction)(&z3zE3289);
                                  KILL(zTransaction)(&z3zE3290);
                                  KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3324);
                                  KILL(zTransaction)(&z3zE3292);
                                  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                  KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                  goto end_block_exception_3369;
                                }
                              }
                              {
                                z2zE3337 = ztx_signing_hash(zFeeMarketTx, z2zE3336, zv);
                                if (have_exception) {
                                  KILL(zTransaction)(&z3zE3279);
                                  KILL(zTransaction)(&z3zE3280);
                                  KILL(zTransaction)(&z3zE3281);
                                  KILL(zTransaction)(&z3zE3282);
                                  KILL(zTransaction)(&z3zE3283);
                                  KILL(zTransaction)(&z3zE3284);
                                  KILL(zTransaction)(&z3zE3285);
                                  KILL(zTransaction)(&z3zE3286);
                                  KILL(zTransaction)(&z3zE3287);
                                  KILL(zTransaction)(&z3zE3288);
                                  KILL(zTransaction)(&z3zE3289);
                                  KILL(zTransaction)(&z3zE3290);
                                  KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3324);
                                  KILL(zTransaction)(&z3zE3292);
                                  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                  KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                  goto end_block_exception_3369;
                                }
                              }
                            }
                            sail_u256 z2zE3338;
                            {
                              z2zE3338 = zrlp_ref_uint_word(zr_f);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE3279);
                                KILL(zTransaction)(&z3zE3280);
                                KILL(zTransaction)(&z3zE3281);
                                KILL(zTransaction)(&z3zE3282);
                                KILL(zTransaction)(&z3zE3283);
                                KILL(zTransaction)(&z3zE3284);
                                KILL(zTransaction)(&z3zE3285);
                                KILL(zTransaction)(&z3zE3286);
                                KILL(zTransaction)(&z3zE3287);
                                KILL(zTransaction)(&z3zE3288);
                                KILL(zTransaction)(&z3zE3289);
                                KILL(zTransaction)(&z3zE3290);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3324);
                                KILL(zTransaction)(&z3zE3292);
                                KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                goto end_block_exception_3369;
                              }
                            }
                            sail_u256 z2zE3339;
                            {
                              z2zE3339 = zrlp_ref_uint_word(zs_f);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE3279);
                                KILL(zTransaction)(&z3zE3280);
                                KILL(zTransaction)(&z3zE3281);
                                KILL(zTransaction)(&z3zE3282);
                                KILL(zTransaction)(&z3zE3283);
                                KILL(zTransaction)(&z3zE3284);
                                KILL(zTransaction)(&z3zE3285);
                                KILL(zTransaction)(&z3zE3286);
                                KILL(zTransaction)(&z3zE3287);
                                KILL(zTransaction)(&z3zE3288);
                                KILL(zTransaction)(&z3zE3289);
                                KILL(zTransaction)(&z3zE3290);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3324);
                                KILL(zTransaction)(&z3zE3292);
                                KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                goto end_block_exception_3369;
                              }
                            }
                            z3zE3292.zaccess_list_address_count = zal_addr_count;
                            COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE3292)->zaccess_list_addresses), zal_addrs);
                            z3zE3292.zaccess_list_slot_count = zal_slot_count;
                            COPY(zz5listz8z5structz0zzStorageKeyz9)(&((&z3zE3292)->zaccess_list_slots), zal_slots);
                            z3zE3292.zauthorizzation_count = UINT64_C(0);
                            zz5listz8z5structz0zzAuthorizzzzationz9 z3zE3294;
                            CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&z3zE3294);
                            COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&((&z3zE3292)->zauthorizzations), z3zE3294);
                            KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z3zE3294);
                            z3zE3292.zblob_hashes = zEMPTY_BLOB_HASHES;
                            z3zE3292.zchain_id = z2zE3326;
                            z3zE3292.zgas_limit = z2zE3327;
                            z3zE3292.zinput_src = z2zE3333;
                            z3zE3292.zis_create = z2zE3329;
                            z3zE3292.zmax_blob_fee = zZERO_WORD;
                            z3zE3292.zmax_fee = z2zE3334;
                            z3zE3292.zmax_priority_fee = z2zE3335;
                            z3zE3292.znonce = z2zE3325;
                            z3zE3292.zpubkey = zpubkey;
                            z3zE3292.zraw = ztx;
                            z3zE3292.zrecipient = z2zE3331;
                            z3zE3292.zsender = zsender;
                            z3zE3292.zsig_r = z2zE3338;
                            z3zE3292.zsig_s = z2zE3339;
                            z3zE3292.zsig_v = zv;
                            z3zE3292.zsigning_hash = z2zE3337;
                            z3zE3292.ztx_type = zFeeMarketTx;
                            z3zE3292.zvalue = z2zE3332;
                            KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                            KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                            goto finish_match_3366;
                          }
                        case_3367: ;
                          sail_match_failure("decode_fee_market_tx");
                        finish_match_3366: ;
                          COPY(zTransaction)(&z3zE3290, z3zE3292);
                          KILL(zTransaction)(&z3zE3292);
                          KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3324);
                          goto finish_match_3364;
                        }
                      case_3365: ;
                        sail_match_failure("decode_fee_market_tx");
                      finish_match_3364: ;
                        COPY(zTransaction)(&z3zE3289, z3zE3290);
                        KILL(zTransaction)(&z3zE3290);
                        goto finish_match_3362;
                      }
                    case_3363: ;
                      sail_match_failure("decode_fee_market_tx");
                    finish_match_3362: ;
                      COPY(zTransaction)(&z3zE3288, z3zE3289);
                      KILL(zTransaction)(&z3zE3289);
                      goto finish_match_3360;
                    }
                  case_3361: ;
                    sail_match_failure("decode_fee_market_tx");
                  finish_match_3360: ;
                    COPY(zTransaction)(&z3zE3287, z3zE3288);
                    KILL(zTransaction)(&z3zE3288);
                    goto finish_match_3358;
                  }
                case_3359: ;
                  sail_match_failure("decode_fee_market_tx");
                finish_match_3358: ;
                  COPY(zTransaction)(&z3zE3286, z3zE3287);
                  KILL(zTransaction)(&z3zE3287);
                  goto finish_match_3356;
                }
              case_3357: ;
                sail_match_failure("decode_fee_market_tx");
              finish_match_3356: ;
                COPY(zTransaction)(&z3zE3285, z3zE3286);
                KILL(zTransaction)(&z3zE3286);
                goto finish_match_3354;
              }
            case_3355: ;
              sail_match_failure("decode_fee_market_tx");
            finish_match_3354: ;
              COPY(zTransaction)(&z3zE3284, z3zE3285);
              KILL(zTransaction)(&z3zE3285);
              goto finish_match_3352;
            }
          case_3353: ;
            sail_match_failure("decode_fee_market_tx");
          finish_match_3352: ;
            COPY(zTransaction)(&z3zE3283, z3zE3284);
            KILL(zTransaction)(&z3zE3284);
            goto finish_match_3350;
          }
        case_3351: ;
          sail_match_failure("decode_fee_market_tx");
        finish_match_3350: ;
          COPY(zTransaction)(&z3zE3282, z3zE3283);
          KILL(zTransaction)(&z3zE3283);
          goto finish_match_3348;
        }
      case_3349: ;
        sail_match_failure("decode_fee_market_tx");
      finish_match_3348: ;
        COPY(zTransaction)(&z3zE3281, z3zE3282);
        KILL(zTransaction)(&z3zE3282);
        goto finish_match_3346;
      }
    case_3347: ;
      sail_match_failure("decode_fee_market_tx");
    finish_match_3346: ;
      COPY(zTransaction)(&z3zE3280, z3zE3281);
      KILL(zTransaction)(&z3zE3281);
      goto finish_match_3344;
    }
  case_3345: ;
    sail_match_failure("decode_fee_market_tx");
  finish_match_3344: ;
    COPY(zTransaction)(&z3zE3279, z3zE3280);
    KILL(zTransaction)(&z3zE3280);
    goto finish_match_3342;
  }
case_3343: ;
  sail_match_failure("decode_fee_market_tx");
finish_match_3342: ;
  COPY(zTransaction)((*(&z8zE201)), z3zE3279);
  KILL(zTransaction)(&z3zE3279);
end_function_3368: ;
  goto end_function_4064;
end_block_exception_3369: ;
  goto end_function_4064;
end_function_4064: ;
}

void zdecode_blob_tx(struct zTransaction *z8zE202, struct zByteSliceFields ztx, struct zByteSliceFields zpubkey, enum zFork zfork, sail_fixed_bytes_20 zsender, struct zRlpCursor zfields)
{
  struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3281;
  {
    z2zE3281 = zrlp_cursor_pop(zfields);
    if (have_exception) {  goto end_block_exception_3341;  }
  }
  struct zTransaction z3zE3246;
  CREATE(zTransaction)(&z3zE3246);
  {
    struct zRlpFieldRef zchain_f;
    zchain_f = z2zE3281.ztup0;
    struct zRlpCursor z1zE22;
    z1zE22 = z2zE3281.ztup1;
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3282;
    {
      z2zE3282 = zrlp_cursor_pop(z1zE22);
      if (have_exception) {
        KILL(zTransaction)(&z3zE3246);
        goto end_block_exception_3341;
      }
    }
    struct zTransaction z3zE3247;
    CREATE(zTransaction)(&z3zE3247);
    {
      struct zRlpFieldRef znonce_f;
      znonce_f = z2zE3282.ztup0;
      struct zRlpCursor z1zE23;
      z1zE23 = z2zE3282.ztup1;
      struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3283;
      {
        z2zE3283 = zrlp_cursor_pop(z1zE23);
        if (have_exception) {
          KILL(zTransaction)(&z3zE3246);
          KILL(zTransaction)(&z3zE3247);
          goto end_block_exception_3341;
        }
      }
      struct zTransaction z3zE3248;
      CREATE(zTransaction)(&z3zE3248);
      {
        struct zRlpFieldRef zmp_f;
        zmp_f = z2zE3283.ztup0;
        struct zRlpCursor z1zE24;
        z1zE24 = z2zE3283.ztup1;
        struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3284;
        {
          z2zE3284 = zrlp_cursor_pop(z1zE24);
          if (have_exception) {
            KILL(zTransaction)(&z3zE3246);
            KILL(zTransaction)(&z3zE3247);
            KILL(zTransaction)(&z3zE3248);
            goto end_block_exception_3341;
          }
        }
        struct zTransaction z3zE3249;
        CREATE(zTransaction)(&z3zE3249);
        {
          struct zRlpFieldRef zmf_f;
          zmf_f = z2zE3284.ztup0;
          struct zRlpCursor z1zE25;
          z1zE25 = z2zE3284.ztup1;
          struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3285;
          {
            z2zE3285 = zrlp_cursor_pop(z1zE25);
            if (have_exception) {
              KILL(zTransaction)(&z3zE3246);
              KILL(zTransaction)(&z3zE3247);
              KILL(zTransaction)(&z3zE3248);
              KILL(zTransaction)(&z3zE3249);
              goto end_block_exception_3341;
            }
          }
          struct zTransaction z3zE3250;
          CREATE(zTransaction)(&z3zE3250);
          {
            struct zRlpFieldRef zgas_f;
            zgas_f = z2zE3285.ztup0;
            struct zRlpCursor z1zE26;
            z1zE26 = z2zE3285.ztup1;
            struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3286;
            {
              z2zE3286 = zrlp_cursor_pop(z1zE26);
              if (have_exception) {
                KILL(zTransaction)(&z3zE3246);
                KILL(zTransaction)(&z3zE3247);
                KILL(zTransaction)(&z3zE3248);
                KILL(zTransaction)(&z3zE3249);
                KILL(zTransaction)(&z3zE3250);
                goto end_block_exception_3341;
              }
            }
            struct zTransaction z3zE3251;
            CREATE(zTransaction)(&z3zE3251);
            {
              struct zRlpFieldRef zto_f;
              zto_f = z2zE3286.ztup0;
              struct zRlpCursor z1zE27;
              z1zE27 = z2zE3286.ztup1;
              struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3287;
              {
                z2zE3287 = zrlp_cursor_pop(z1zE27);
                if (have_exception) {
                  KILL(zTransaction)(&z3zE3246);
                  KILL(zTransaction)(&z3zE3247);
                  KILL(zTransaction)(&z3zE3248);
                  KILL(zTransaction)(&z3zE3249);
                  KILL(zTransaction)(&z3zE3250);
                  KILL(zTransaction)(&z3zE3251);
                  goto end_block_exception_3341;
                }
              }
              struct zTransaction z3zE3252;
              CREATE(zTransaction)(&z3zE3252);
              {
                struct zRlpFieldRef zvalue_f;
                zvalue_f = z2zE3287.ztup0;
                struct zRlpCursor z1zE28;
                z1zE28 = z2zE3287.ztup1;
                struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3288;
                {
                  z2zE3288 = zrlp_cursor_pop(z1zE28);
                  if (have_exception) {
                    KILL(zTransaction)(&z3zE3246);
                    KILL(zTransaction)(&z3zE3247);
                    KILL(zTransaction)(&z3zE3248);
                    KILL(zTransaction)(&z3zE3249);
                    KILL(zTransaction)(&z3zE3250);
                    KILL(zTransaction)(&z3zE3251);
                    KILL(zTransaction)(&z3zE3252);
                    goto end_block_exception_3341;
                  }
                }
                struct zTransaction z3zE3253;
                CREATE(zTransaction)(&z3zE3253);
                {
                  struct zRlpFieldRef zdata_f;
                  zdata_f = z2zE3288.ztup0;
                  struct zRlpCursor z1zE29;
                  z1zE29 = z2zE3288.ztup1;
                  struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3289;
                  {
                    z2zE3289 = zrlp_cursor_pop(z1zE29);
                    if (have_exception) {
                      KILL(zTransaction)(&z3zE3246);
                      KILL(zTransaction)(&z3zE3247);
                      KILL(zTransaction)(&z3zE3248);
                      KILL(zTransaction)(&z3zE3249);
                      KILL(zTransaction)(&z3zE3250);
                      KILL(zTransaction)(&z3zE3251);
                      KILL(zTransaction)(&z3zE3252);
                      KILL(zTransaction)(&z3zE3253);
                      goto end_block_exception_3341;
                    }
                  }
                  struct zTransaction z3zE3254;
                  CREATE(zTransaction)(&z3zE3254);
                  {
                    struct zRlpFieldRef zal_f;
                    zal_f = z2zE3289.ztup0;
                    struct zRlpCursor z1zE30;
                    z1zE30 = z2zE3289.ztup1;
                    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3290;
                    {
                      z2zE3290 = zrlp_cursor_pop(z1zE30);
                      if (have_exception) {
                        KILL(zTransaction)(&z3zE3246);
                        KILL(zTransaction)(&z3zE3247);
                        KILL(zTransaction)(&z3zE3248);
                        KILL(zTransaction)(&z3zE3249);
                        KILL(zTransaction)(&z3zE3250);
                        KILL(zTransaction)(&z3zE3251);
                        KILL(zTransaction)(&z3zE3252);
                        KILL(zTransaction)(&z3zE3253);
                        KILL(zTransaction)(&z3zE3254);
                        goto end_block_exception_3341;
                      }
                    }
                    struct zTransaction z3zE3255;
                    CREATE(zTransaction)(&z3zE3255);
                    {
                      struct zRlpFieldRef zmbf_f;
                      zmbf_f = z2zE3290.ztup0;
                      struct zRlpCursor z1zE31;
                      z1zE31 = z2zE3290.ztup1;
                      struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3291;
                      {
                        z2zE3291 = zrlp_cursor_pop(z1zE31);
                        if (have_exception) {
                          KILL(zTransaction)(&z3zE3246);
                          KILL(zTransaction)(&z3zE3247);
                          KILL(zTransaction)(&z3zE3248);
                          KILL(zTransaction)(&z3zE3249);
                          KILL(zTransaction)(&z3zE3250);
                          KILL(zTransaction)(&z3zE3251);
                          KILL(zTransaction)(&z3zE3252);
                          KILL(zTransaction)(&z3zE3253);
                          KILL(zTransaction)(&z3zE3254);
                          KILL(zTransaction)(&z3zE3255);
                          goto end_block_exception_3341;
                        }
                      }
                      struct zTransaction z3zE3256;
                      CREATE(zTransaction)(&z3zE3256);
                      {
                        struct zRlpFieldRef zbh_f;
                        zbh_f = z2zE3291.ztup0;
                        struct zRlpCursor z1zE32;
                        z1zE32 = z2zE3291.ztup1;
                        struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3292;
                        {
                          z2zE3292 = zrlp_cursor_pop(z1zE32);
                          if (have_exception) {
                            KILL(zTransaction)(&z3zE3246);
                            KILL(zTransaction)(&z3zE3247);
                            KILL(zTransaction)(&z3zE3248);
                            KILL(zTransaction)(&z3zE3249);
                            KILL(zTransaction)(&z3zE3250);
                            KILL(zTransaction)(&z3zE3251);
                            KILL(zTransaction)(&z3zE3252);
                            KILL(zTransaction)(&z3zE3253);
                            KILL(zTransaction)(&z3zE3254);
                            KILL(zTransaction)(&z3zE3255);
                            KILL(zTransaction)(&z3zE3256);
                            goto end_block_exception_3341;
                          }
                        }
                        struct zTransaction z3zE3257;
                        CREATE(zTransaction)(&z3zE3257);
                        {
                          struct zRlpFieldRef zv_f;
                          zv_f = z2zE3292.ztup0;
                          struct zRlpCursor z1zE33;
                          z1zE33 = z2zE3292.ztup1;
                          struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3293;
                          {
                            z2zE3293 = zrlp_cursor_pop(z1zE33);
                            if (have_exception) {
                              KILL(zTransaction)(&z3zE3246);
                              KILL(zTransaction)(&z3zE3247);
                              KILL(zTransaction)(&z3zE3248);
                              KILL(zTransaction)(&z3zE3249);
                              KILL(zTransaction)(&z3zE3250);
                              KILL(zTransaction)(&z3zE3251);
                              KILL(zTransaction)(&z3zE3252);
                              KILL(zTransaction)(&z3zE3253);
                              KILL(zTransaction)(&z3zE3254);
                              KILL(zTransaction)(&z3zE3255);
                              KILL(zTransaction)(&z3zE3256);
                              KILL(zTransaction)(&z3zE3257);
                              goto end_block_exception_3341;
                            }
                          }
                          struct zTransaction z3zE3258;
                          CREATE(zTransaction)(&z3zE3258);
                          {
                            struct zRlpFieldRef zr_f;
                            zr_f = z2zE3293.ztup0;
                            struct zRlpCursor z1zE34;
                            z1zE34 = z2zE3293.ztup1;
                            struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3294;
                            {
                              z2zE3294 = zrlp_cursor_pop(z1zE34);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE3246);
                                KILL(zTransaction)(&z3zE3247);
                                KILL(zTransaction)(&z3zE3248);
                                KILL(zTransaction)(&z3zE3249);
                                KILL(zTransaction)(&z3zE3250);
                                KILL(zTransaction)(&z3zE3251);
                                KILL(zTransaction)(&z3zE3252);
                                KILL(zTransaction)(&z3zE3253);
                                KILL(zTransaction)(&z3zE3254);
                                KILL(zTransaction)(&z3zE3255);
                                KILL(zTransaction)(&z3zE3256);
                                KILL(zTransaction)(&z3zE3257);
                                KILL(zTransaction)(&z3zE3258);
                                goto end_block_exception_3341;
                              }
                            }
                            struct zTransaction z3zE3259;
                            CREATE(zTransaction)(&z3zE3259);
                            {
                              struct zRlpFieldRef zs_f;
                              zs_f = z2zE3294.ztup0;
                              struct zRlpCursor z1zE35;
                              z1zE35 = z2zE3294.ztup1;
                              unit z3zE3260;
                              {
                                z3zE3260 = zrlp_cursor_expect_end(z1zE35);
                                if (have_exception) {
                                  KILL(zTransaction)(&z3zE3246);
                                  KILL(zTransaction)(&z3zE3247);
                                  KILL(zTransaction)(&z3zE3248);
                                  KILL(zTransaction)(&z3zE3249);
                                  KILL(zTransaction)(&z3zE3250);
                                  KILL(zTransaction)(&z3zE3251);
                                  KILL(zTransaction)(&z3zE3252);
                                  KILL(zTransaction)(&z3zE3253);
                                  KILL(zTransaction)(&z3zE3254);
                                  KILL(zTransaction)(&z3zE3255);
                                  KILL(zTransaction)(&z3zE3256);
                                  KILL(zTransaction)(&z3zE3257);
                                  KILL(zTransaction)(&z3zE3258);
                                  KILL(zTransaction)(&z3zE3259);
                                  goto end_block_exception_3341;
                                }
                              }
                              sail_u256 zv;
                              {
                                zv = zrlp_ref_word(zv_f);
                                if (have_exception) {
                                  KILL(zTransaction)(&z3zE3246);
                                  KILL(zTransaction)(&z3zE3247);
                                  KILL(zTransaction)(&z3zE3248);
                                  KILL(zTransaction)(&z3zE3249);
                                  KILL(zTransaction)(&z3zE3250);
                                  KILL(zTransaction)(&z3zE3251);
                                  KILL(zTransaction)(&z3zE3252);
                                  KILL(zTransaction)(&z3zE3253);
                                  KILL(zTransaction)(&z3zE3254);
                                  KILL(zTransaction)(&z3zE3255);
                                  KILL(zTransaction)(&z3zE3256);
                                  KILL(zTransaction)(&z3zE3257);
                                  KILL(zTransaction)(&z3zE3258);
                                  KILL(zTransaction)(&z3zE3259);
                                  goto end_block_exception_3341;
                                }
                              }
                              struct ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9 z2zE3295;
                              CREATE(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3295);
                              {
                                zdecode_access_list(&z2zE3295, zal_f);
                                if (have_exception) {
                                  KILL(zTransaction)(&z3zE3246);
                                  KILL(zTransaction)(&z3zE3247);
                                  KILL(zTransaction)(&z3zE3248);
                                  KILL(zTransaction)(&z3zE3249);
                                  KILL(zTransaction)(&z3zE3250);
                                  KILL(zTransaction)(&z3zE3251);
                                  KILL(zTransaction)(&z3zE3252);
                                  KILL(zTransaction)(&z3zE3253);
                                  KILL(zTransaction)(&z3zE3254);
                                  KILL(zTransaction)(&z3zE3255);
                                  KILL(zTransaction)(&z3zE3256);
                                  KILL(zTransaction)(&z3zE3257);
                                  KILL(zTransaction)(&z3zE3258);
                                  KILL(zTransaction)(&z3zE3259);
                                  KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3295);
                                  goto end_block_exception_3341;
                                }
                              }
                              struct zTransaction z3zE3261;
                              CREATE(zTransaction)(&z3zE3261);
                              {
                                zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 zal_addrs;
                                CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs, z2zE3295.ztup0);
                                zz5listz8z5structz0zzStorageKeyz9 zal_slots;
                                CREATE(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                COPY(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots, z2zE3295.ztup1);
                                uint64_t zal_addr_count;
                                zal_addr_count = z2zE3295.ztup2;
                                uint64_t zal_slot_count;
                                zal_slot_count = z2zE3295.ztup3;
                                struct zBlobHashes zblob_hashes;
                                {
                                  zblob_hashes = zdecode_blob_hashes(zbh_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE3246);
                                    KILL(zTransaction)(&z3zE3247);
                                    KILL(zTransaction)(&z3zE3248);
                                    KILL(zTransaction)(&z3zE3249);
                                    KILL(zTransaction)(&z3zE3250);
                                    KILL(zTransaction)(&z3zE3251);
                                    KILL(zTransaction)(&z3zE3252);
                                    KILL(zTransaction)(&z3zE3253);
                                    KILL(zTransaction)(&z3zE3254);
                                    KILL(zTransaction)(&z3zE3255);
                                    KILL(zTransaction)(&z3zE3256);
                                    KILL(zTransaction)(&z3zE3257);
                                    KILL(zTransaction)(&z3zE3258);
                                    KILL(zTransaction)(&z3zE3259);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3295);
                                    KILL(zTransaction)(&z3zE3261);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    goto end_block_exception_3341;
                                  }
                                }
                                sail_u256 z2zE3296;
                                {
                                  z2zE3296 = zrlp_ref_uint_word(znonce_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE3246);
                                    KILL(zTransaction)(&z3zE3247);
                                    KILL(zTransaction)(&z3zE3248);
                                    KILL(zTransaction)(&z3zE3249);
                                    KILL(zTransaction)(&z3zE3250);
                                    KILL(zTransaction)(&z3zE3251);
                                    KILL(zTransaction)(&z3zE3252);
                                    KILL(zTransaction)(&z3zE3253);
                                    KILL(zTransaction)(&z3zE3254);
                                    KILL(zTransaction)(&z3zE3255);
                                    KILL(zTransaction)(&z3zE3256);
                                    KILL(zTransaction)(&z3zE3257);
                                    KILL(zTransaction)(&z3zE3258);
                                    KILL(zTransaction)(&z3zE3259);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3295);
                                    KILL(zTransaction)(&z3zE3261);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    goto end_block_exception_3341;
                                  }
                                }
                                uint64_t z2zE3297;
                                {
                                  z2zE3297 = zrlp_ref_uint64(zchain_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE3246);
                                    KILL(zTransaction)(&z3zE3247);
                                    KILL(zTransaction)(&z3zE3248);
                                    KILL(zTransaction)(&z3zE3249);
                                    KILL(zTransaction)(&z3zE3250);
                                    KILL(zTransaction)(&z3zE3251);
                                    KILL(zTransaction)(&z3zE3252);
                                    KILL(zTransaction)(&z3zE3253);
                                    KILL(zTransaction)(&z3zE3254);
                                    KILL(zTransaction)(&z3zE3255);
                                    KILL(zTransaction)(&z3zE3256);
                                    KILL(zTransaction)(&z3zE3257);
                                    KILL(zTransaction)(&z3zE3258);
                                    KILL(zTransaction)(&z3zE3259);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3295);
                                    KILL(zTransaction)(&z3zE3261);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    goto end_block_exception_3341;
                                  }
                                }
                                uint64_t z2zE3298;
                                {
                                  z2zE3298 = zrlp_ref_gas(zgas_f, zfork);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE3246);
                                    KILL(zTransaction)(&z3zE3247);
                                    KILL(zTransaction)(&z3zE3248);
                                    KILL(zTransaction)(&z3zE3249);
                                    KILL(zTransaction)(&z3zE3250);
                                    KILL(zTransaction)(&z3zE3251);
                                    KILL(zTransaction)(&z3zE3252);
                                    KILL(zTransaction)(&z3zE3253);
                                    KILL(zTransaction)(&z3zE3254);
                                    KILL(zTransaction)(&z3zE3255);
                                    KILL(zTransaction)(&z3zE3256);
                                    KILL(zTransaction)(&z3zE3257);
                                    KILL(zTransaction)(&z3zE3258);
                                    KILL(zTransaction)(&z3zE3259);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3295);
                                    KILL(zTransaction)(&z3zE3261);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    goto end_block_exception_3341;
                                  }
                                }
                                bool z2zE3300;
                                {
                                  uint64_t z2zE3299;
                                  z2zE3299 = zto_f.zcontent_len;
                                  z2zE3300 = (z2zE3299 == UINT64_C(0));
                                }
                                sail_fixed_bytes_20 z2zE3302;
                                {
                                  sail_u256 z2zE3301;
                                  {
                                    z2zE3301 = zrlp_ref_word(zto_f);
                                    if (have_exception) {
                                      KILL(zTransaction)(&z3zE3246);
                                      KILL(zTransaction)(&z3zE3247);
                                      KILL(zTransaction)(&z3zE3248);
                                      KILL(zTransaction)(&z3zE3249);
                                      KILL(zTransaction)(&z3zE3250);
                                      KILL(zTransaction)(&z3zE3251);
                                      KILL(zTransaction)(&z3zE3252);
                                      KILL(zTransaction)(&z3zE3253);
                                      KILL(zTransaction)(&z3zE3254);
                                      KILL(zTransaction)(&z3zE3255);
                                      KILL(zTransaction)(&z3zE3256);
                                      KILL(zTransaction)(&z3zE3257);
                                      KILL(zTransaction)(&z3zE3258);
                                      KILL(zTransaction)(&z3zE3259);
                                      KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3295);
                                      KILL(zTransaction)(&z3zE3261);
                                      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                      KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                      goto end_block_exception_3341;
                                    }
                                  }
                                  z2zE3302 = zword_to_address(z2zE3301);
                                }
                                sail_u256 z2zE3303;
                                {
                                  z2zE3303 = zrlp_ref_uint_word(zvalue_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE3246);
                                    KILL(zTransaction)(&z3zE3247);
                                    KILL(zTransaction)(&z3zE3248);
                                    KILL(zTransaction)(&z3zE3249);
                                    KILL(zTransaction)(&z3zE3250);
                                    KILL(zTransaction)(&z3zE3251);
                                    KILL(zTransaction)(&z3zE3252);
                                    KILL(zTransaction)(&z3zE3253);
                                    KILL(zTransaction)(&z3zE3254);
                                    KILL(zTransaction)(&z3zE3255);
                                    KILL(zTransaction)(&z3zE3256);
                                    KILL(zTransaction)(&z3zE3257);
                                    KILL(zTransaction)(&z3zE3258);
                                    KILL(zTransaction)(&z3zE3259);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3295);
                                    KILL(zTransaction)(&z3zE3261);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    goto end_block_exception_3341;
                                  }
                                }
                                struct zByteSliceFields z2zE3304;
                                {
                                  z2zE3304 = ztx_input_span(zdata_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE3246);
                                    KILL(zTransaction)(&z3zE3247);
                                    KILL(zTransaction)(&z3zE3248);
                                    KILL(zTransaction)(&z3zE3249);
                                    KILL(zTransaction)(&z3zE3250);
                                    KILL(zTransaction)(&z3zE3251);
                                    KILL(zTransaction)(&z3zE3252);
                                    KILL(zTransaction)(&z3zE3253);
                                    KILL(zTransaction)(&z3zE3254);
                                    KILL(zTransaction)(&z3zE3255);
                                    KILL(zTransaction)(&z3zE3256);
                                    KILL(zTransaction)(&z3zE3257);
                                    KILL(zTransaction)(&z3zE3258);
                                    KILL(zTransaction)(&z3zE3259);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3295);
                                    KILL(zTransaction)(&z3zE3261);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    goto end_block_exception_3341;
                                  }
                                }
                                sail_u256 z2zE3305;
                                {
                                  z2zE3305 = zrlp_ref_uint_word(zmf_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE3246);
                                    KILL(zTransaction)(&z3zE3247);
                                    KILL(zTransaction)(&z3zE3248);
                                    KILL(zTransaction)(&z3zE3249);
                                    KILL(zTransaction)(&z3zE3250);
                                    KILL(zTransaction)(&z3zE3251);
                                    KILL(zTransaction)(&z3zE3252);
                                    KILL(zTransaction)(&z3zE3253);
                                    KILL(zTransaction)(&z3zE3254);
                                    KILL(zTransaction)(&z3zE3255);
                                    KILL(zTransaction)(&z3zE3256);
                                    KILL(zTransaction)(&z3zE3257);
                                    KILL(zTransaction)(&z3zE3258);
                                    KILL(zTransaction)(&z3zE3259);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3295);
                                    KILL(zTransaction)(&z3zE3261);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    goto end_block_exception_3341;
                                  }
                                }
                                sail_u256 z2zE3306;
                                {
                                  z2zE3306 = zrlp_ref_uint_word(zmbf_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE3246);
                                    KILL(zTransaction)(&z3zE3247);
                                    KILL(zTransaction)(&z3zE3248);
                                    KILL(zTransaction)(&z3zE3249);
                                    KILL(zTransaction)(&z3zE3250);
                                    KILL(zTransaction)(&z3zE3251);
                                    KILL(zTransaction)(&z3zE3252);
                                    KILL(zTransaction)(&z3zE3253);
                                    KILL(zTransaction)(&z3zE3254);
                                    KILL(zTransaction)(&z3zE3255);
                                    KILL(zTransaction)(&z3zE3256);
                                    KILL(zTransaction)(&z3zE3257);
                                    KILL(zTransaction)(&z3zE3258);
                                    KILL(zTransaction)(&z3zE3259);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3295);
                                    KILL(zTransaction)(&z3zE3261);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    goto end_block_exception_3341;
                                  }
                                }
                                sail_u256 z2zE3307;
                                {
                                  z2zE3307 = zrlp_ref_uint_word(zmp_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE3246);
                                    KILL(zTransaction)(&z3zE3247);
                                    KILL(zTransaction)(&z3zE3248);
                                    KILL(zTransaction)(&z3zE3249);
                                    KILL(zTransaction)(&z3zE3250);
                                    KILL(zTransaction)(&z3zE3251);
                                    KILL(zTransaction)(&z3zE3252);
                                    KILL(zTransaction)(&z3zE3253);
                                    KILL(zTransaction)(&z3zE3254);
                                    KILL(zTransaction)(&z3zE3255);
                                    KILL(zTransaction)(&z3zE3256);
                                    KILL(zTransaction)(&z3zE3257);
                                    KILL(zTransaction)(&z3zE3258);
                                    KILL(zTransaction)(&z3zE3259);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3295);
                                    KILL(zTransaction)(&z3zE3261);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    goto end_block_exception_3341;
                                  }
                                }
                                sail_fixed_bytes_32 z2zE3309;
                                {
                                  struct zByteSliceFields z2zE3308;
                                  {
                                    z2zE3308 = ztx_sig_span(zchain_f, zv_f);
                                    if (have_exception) {
                                      KILL(zTransaction)(&z3zE3246);
                                      KILL(zTransaction)(&z3zE3247);
                                      KILL(zTransaction)(&z3zE3248);
                                      KILL(zTransaction)(&z3zE3249);
                                      KILL(zTransaction)(&z3zE3250);
                                      KILL(zTransaction)(&z3zE3251);
                                      KILL(zTransaction)(&z3zE3252);
                                      KILL(zTransaction)(&z3zE3253);
                                      KILL(zTransaction)(&z3zE3254);
                                      KILL(zTransaction)(&z3zE3255);
                                      KILL(zTransaction)(&z3zE3256);
                                      KILL(zTransaction)(&z3zE3257);
                                      KILL(zTransaction)(&z3zE3258);
                                      KILL(zTransaction)(&z3zE3259);
                                      KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3295);
                                      KILL(zTransaction)(&z3zE3261);
                                      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                      KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                      goto end_block_exception_3341;
                                    }
                                  }
                                  {
                                    z2zE3309 = ztx_signing_hash(zBlobTx, z2zE3308, zv);
                                    if (have_exception) {
                                      KILL(zTransaction)(&z3zE3246);
                                      KILL(zTransaction)(&z3zE3247);
                                      KILL(zTransaction)(&z3zE3248);
                                      KILL(zTransaction)(&z3zE3249);
                                      KILL(zTransaction)(&z3zE3250);
                                      KILL(zTransaction)(&z3zE3251);
                                      KILL(zTransaction)(&z3zE3252);
                                      KILL(zTransaction)(&z3zE3253);
                                      KILL(zTransaction)(&z3zE3254);
                                      KILL(zTransaction)(&z3zE3255);
                                      KILL(zTransaction)(&z3zE3256);
                                      KILL(zTransaction)(&z3zE3257);
                                      KILL(zTransaction)(&z3zE3258);
                                      KILL(zTransaction)(&z3zE3259);
                                      KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3295);
                                      KILL(zTransaction)(&z3zE3261);
                                      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                      KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                      goto end_block_exception_3341;
                                    }
                                  }
                                }
                                sail_u256 z2zE3310;
                                {
                                  z2zE3310 = zrlp_ref_uint_word(zr_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE3246);
                                    KILL(zTransaction)(&z3zE3247);
                                    KILL(zTransaction)(&z3zE3248);
                                    KILL(zTransaction)(&z3zE3249);
                                    KILL(zTransaction)(&z3zE3250);
                                    KILL(zTransaction)(&z3zE3251);
                                    KILL(zTransaction)(&z3zE3252);
                                    KILL(zTransaction)(&z3zE3253);
                                    KILL(zTransaction)(&z3zE3254);
                                    KILL(zTransaction)(&z3zE3255);
                                    KILL(zTransaction)(&z3zE3256);
                                    KILL(zTransaction)(&z3zE3257);
                                    KILL(zTransaction)(&z3zE3258);
                                    KILL(zTransaction)(&z3zE3259);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3295);
                                    KILL(zTransaction)(&z3zE3261);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    goto end_block_exception_3341;
                                  }
                                }
                                sail_u256 z2zE3311;
                                {
                                  z2zE3311 = zrlp_ref_uint_word(zs_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE3246);
                                    KILL(zTransaction)(&z3zE3247);
                                    KILL(zTransaction)(&z3zE3248);
                                    KILL(zTransaction)(&z3zE3249);
                                    KILL(zTransaction)(&z3zE3250);
                                    KILL(zTransaction)(&z3zE3251);
                                    KILL(zTransaction)(&z3zE3252);
                                    KILL(zTransaction)(&z3zE3253);
                                    KILL(zTransaction)(&z3zE3254);
                                    KILL(zTransaction)(&z3zE3255);
                                    KILL(zTransaction)(&z3zE3256);
                                    KILL(zTransaction)(&z3zE3257);
                                    KILL(zTransaction)(&z3zE3258);
                                    KILL(zTransaction)(&z3zE3259);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3295);
                                    KILL(zTransaction)(&z3zE3261);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    goto end_block_exception_3341;
                                  }
                                }
                                z3zE3261.zaccess_list_address_count = zal_addr_count;
                                COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE3261)->zaccess_list_addresses), zal_addrs);
                                z3zE3261.zaccess_list_slot_count = zal_slot_count;
                                COPY(zz5listz8z5structz0zzStorageKeyz9)(&((&z3zE3261)->zaccess_list_slots), zal_slots);
                                z3zE3261.zauthorizzation_count = UINT64_C(0);
                                zz5listz8z5structz0zzAuthorizzzzationz9 z3zE3263;
                                CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&z3zE3263);
                                COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&((&z3zE3261)->zauthorizzations), z3zE3263);
                                KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z3zE3263);
                                z3zE3261.zblob_hashes = zblob_hashes;
                                z3zE3261.zchain_id = z2zE3297;
                                z3zE3261.zgas_limit = z2zE3298;
                                z3zE3261.zinput_src = z2zE3304;
                                z3zE3261.zis_create = z2zE3300;
                                z3zE3261.zmax_blob_fee = z2zE3306;
                                z3zE3261.zmax_fee = z2zE3305;
                                z3zE3261.zmax_priority_fee = z2zE3307;
                                z3zE3261.znonce = z2zE3296;
                                z3zE3261.zpubkey = zpubkey;
                                z3zE3261.zraw = ztx;
                                z3zE3261.zrecipient = z2zE3302;
                                z3zE3261.zsender = zsender;
                                z3zE3261.zsig_r = z2zE3310;
                                z3zE3261.zsig_s = z2zE3311;
                                z3zE3261.zsig_v = zv;
                                z3zE3261.zsigning_hash = z2zE3309;
                                z3zE3261.ztx_type = zBlobTx;
                                z3zE3261.zvalue = z2zE3303;
                                KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                goto finish_match_3338;
                              }
                            case_3339: ;
                              sail_match_failure("decode_blob_tx");
                            finish_match_3338: ;
                              COPY(zTransaction)(&z3zE3259, z3zE3261);
                              KILL(zTransaction)(&z3zE3261);
                              KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3295);
                              goto finish_match_3336;
                            }
                          case_3337: ;
                            sail_match_failure("decode_blob_tx");
                          finish_match_3336: ;
                            COPY(zTransaction)(&z3zE3258, z3zE3259);
                            KILL(zTransaction)(&z3zE3259);
                            goto finish_match_3334;
                          }
                        case_3335: ;
                          sail_match_failure("decode_blob_tx");
                        finish_match_3334: ;
                          COPY(zTransaction)(&z3zE3257, z3zE3258);
                          KILL(zTransaction)(&z3zE3258);
                          goto finish_match_3332;
                        }
                      case_3333: ;
                        sail_match_failure("decode_blob_tx");
                      finish_match_3332: ;
                        COPY(zTransaction)(&z3zE3256, z3zE3257);
                        KILL(zTransaction)(&z3zE3257);
                        goto finish_match_3330;
                      }
                    case_3331: ;
                      sail_match_failure("decode_blob_tx");
                    finish_match_3330: ;
                      COPY(zTransaction)(&z3zE3255, z3zE3256);
                      KILL(zTransaction)(&z3zE3256);
                      goto finish_match_3328;
                    }
                  case_3329: ;
                    sail_match_failure("decode_blob_tx");
                  finish_match_3328: ;
                    COPY(zTransaction)(&z3zE3254, z3zE3255);
                    KILL(zTransaction)(&z3zE3255);
                    goto finish_match_3326;
                  }
                case_3327: ;
                  sail_match_failure("decode_blob_tx");
                finish_match_3326: ;
                  COPY(zTransaction)(&z3zE3253, z3zE3254);
                  KILL(zTransaction)(&z3zE3254);
                  goto finish_match_3324;
                }
              case_3325: ;
                sail_match_failure("decode_blob_tx");
              finish_match_3324: ;
                COPY(zTransaction)(&z3zE3252, z3zE3253);
                KILL(zTransaction)(&z3zE3253);
                goto finish_match_3322;
              }
            case_3323: ;
              sail_match_failure("decode_blob_tx");
            finish_match_3322: ;
              COPY(zTransaction)(&z3zE3251, z3zE3252);
              KILL(zTransaction)(&z3zE3252);
              goto finish_match_3320;
            }
          case_3321: ;
            sail_match_failure("decode_blob_tx");
          finish_match_3320: ;
            COPY(zTransaction)(&z3zE3250, z3zE3251);
            KILL(zTransaction)(&z3zE3251);
            goto finish_match_3318;
          }
        case_3319: ;
          sail_match_failure("decode_blob_tx");
        finish_match_3318: ;
          COPY(zTransaction)(&z3zE3249, z3zE3250);
          KILL(zTransaction)(&z3zE3250);
          goto finish_match_3316;
        }
      case_3317: ;
        sail_match_failure("decode_blob_tx");
      finish_match_3316: ;
        COPY(zTransaction)(&z3zE3248, z3zE3249);
        KILL(zTransaction)(&z3zE3249);
        goto finish_match_3314;
      }
    case_3315: ;
      sail_match_failure("decode_blob_tx");
    finish_match_3314: ;
      COPY(zTransaction)(&z3zE3247, z3zE3248);
      KILL(zTransaction)(&z3zE3248);
      goto finish_match_3312;
    }
  case_3313: ;
    sail_match_failure("decode_blob_tx");
  finish_match_3312: ;
    COPY(zTransaction)(&z3zE3246, z3zE3247);
    KILL(zTransaction)(&z3zE3247);
    goto finish_match_3310;
  }
case_3311: ;
  sail_match_failure("decode_blob_tx");
finish_match_3310: ;
  COPY(zTransaction)((*(&z8zE202)), z3zE3246);
  KILL(zTransaction)(&z3zE3246);
end_function_3340: ;
  goto end_function_4063;
end_block_exception_3341: ;
  goto end_function_4063;
end_function_4063: ;
}

void zdecode_set_code_tx(struct zTransaction *z8zE203, struct zByteSliceFields ztx, struct zByteSliceFields zpubkey, enum zFork zfork, sail_fixed_bytes_20 zsender, struct zRlpCursor zfields)
{
  struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3250;
  {
    z2zE3250 = zrlp_cursor_pop(zfields);
    if (have_exception) {  goto end_block_exception_3309;  }
  }
  struct zTransaction z3zE3214;
  CREATE(zTransaction)(&z3zE3214);
  {
    struct zRlpFieldRef zchain_f;
    zchain_f = z2zE3250.ztup0;
    struct zRlpCursor z1zE9;
    z1zE9 = z2zE3250.ztup1;
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3251;
    {
      z2zE3251 = zrlp_cursor_pop(z1zE9);
      if (have_exception) {
        KILL(zTransaction)(&z3zE3214);
        goto end_block_exception_3309;
      }
    }
    struct zTransaction z3zE3215;
    CREATE(zTransaction)(&z3zE3215);
    {
      struct zRlpFieldRef znonce_f;
      znonce_f = z2zE3251.ztup0;
      struct zRlpCursor z1zE10;
      z1zE10 = z2zE3251.ztup1;
      struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3252;
      {
        z2zE3252 = zrlp_cursor_pop(z1zE10);
        if (have_exception) {
          KILL(zTransaction)(&z3zE3214);
          KILL(zTransaction)(&z3zE3215);
          goto end_block_exception_3309;
        }
      }
      struct zTransaction z3zE3216;
      CREATE(zTransaction)(&z3zE3216);
      {
        struct zRlpFieldRef zmp_f;
        zmp_f = z2zE3252.ztup0;
        struct zRlpCursor z1zE11;
        z1zE11 = z2zE3252.ztup1;
        struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3253;
        {
          z2zE3253 = zrlp_cursor_pop(z1zE11);
          if (have_exception) {
            KILL(zTransaction)(&z3zE3214);
            KILL(zTransaction)(&z3zE3215);
            KILL(zTransaction)(&z3zE3216);
            goto end_block_exception_3309;
          }
        }
        struct zTransaction z3zE3217;
        CREATE(zTransaction)(&z3zE3217);
        {
          struct zRlpFieldRef zmf_f;
          zmf_f = z2zE3253.ztup0;
          struct zRlpCursor z1zE12;
          z1zE12 = z2zE3253.ztup1;
          struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3254;
          {
            z2zE3254 = zrlp_cursor_pop(z1zE12);
            if (have_exception) {
              KILL(zTransaction)(&z3zE3214);
              KILL(zTransaction)(&z3zE3215);
              KILL(zTransaction)(&z3zE3216);
              KILL(zTransaction)(&z3zE3217);
              goto end_block_exception_3309;
            }
          }
          struct zTransaction z3zE3218;
          CREATE(zTransaction)(&z3zE3218);
          {
            struct zRlpFieldRef zgas_f;
            zgas_f = z2zE3254.ztup0;
            struct zRlpCursor z1zE13;
            z1zE13 = z2zE3254.ztup1;
            struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3255;
            {
              z2zE3255 = zrlp_cursor_pop(z1zE13);
              if (have_exception) {
                KILL(zTransaction)(&z3zE3214);
                KILL(zTransaction)(&z3zE3215);
                KILL(zTransaction)(&z3zE3216);
                KILL(zTransaction)(&z3zE3217);
                KILL(zTransaction)(&z3zE3218);
                goto end_block_exception_3309;
              }
            }
            struct zTransaction z3zE3219;
            CREATE(zTransaction)(&z3zE3219);
            {
              struct zRlpFieldRef zto_f;
              zto_f = z2zE3255.ztup0;
              struct zRlpCursor z1zE14;
              z1zE14 = z2zE3255.ztup1;
              struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3256;
              {
                z2zE3256 = zrlp_cursor_pop(z1zE14);
                if (have_exception) {
                  KILL(zTransaction)(&z3zE3214);
                  KILL(zTransaction)(&z3zE3215);
                  KILL(zTransaction)(&z3zE3216);
                  KILL(zTransaction)(&z3zE3217);
                  KILL(zTransaction)(&z3zE3218);
                  KILL(zTransaction)(&z3zE3219);
                  goto end_block_exception_3309;
                }
              }
              struct zTransaction z3zE3220;
              CREATE(zTransaction)(&z3zE3220);
              {
                struct zRlpFieldRef zvalue_f;
                zvalue_f = z2zE3256.ztup0;
                struct zRlpCursor z1zE15;
                z1zE15 = z2zE3256.ztup1;
                struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3257;
                {
                  z2zE3257 = zrlp_cursor_pop(z1zE15);
                  if (have_exception) {
                    KILL(zTransaction)(&z3zE3214);
                    KILL(zTransaction)(&z3zE3215);
                    KILL(zTransaction)(&z3zE3216);
                    KILL(zTransaction)(&z3zE3217);
                    KILL(zTransaction)(&z3zE3218);
                    KILL(zTransaction)(&z3zE3219);
                    KILL(zTransaction)(&z3zE3220);
                    goto end_block_exception_3309;
                  }
                }
                struct zTransaction z3zE3221;
                CREATE(zTransaction)(&z3zE3221);
                {
                  struct zRlpFieldRef zdata_f;
                  zdata_f = z2zE3257.ztup0;
                  struct zRlpCursor z1zE16;
                  z1zE16 = z2zE3257.ztup1;
                  struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3258;
                  {
                    z2zE3258 = zrlp_cursor_pop(z1zE16);
                    if (have_exception) {
                      KILL(zTransaction)(&z3zE3214);
                      KILL(zTransaction)(&z3zE3215);
                      KILL(zTransaction)(&z3zE3216);
                      KILL(zTransaction)(&z3zE3217);
                      KILL(zTransaction)(&z3zE3218);
                      KILL(zTransaction)(&z3zE3219);
                      KILL(zTransaction)(&z3zE3220);
                      KILL(zTransaction)(&z3zE3221);
                      goto end_block_exception_3309;
                    }
                  }
                  struct zTransaction z3zE3222;
                  CREATE(zTransaction)(&z3zE3222);
                  {
                    struct zRlpFieldRef zal_f;
                    zal_f = z2zE3258.ztup0;
                    struct zRlpCursor z1zE17;
                    z1zE17 = z2zE3258.ztup1;
                    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3259;
                    {
                      z2zE3259 = zrlp_cursor_pop(z1zE17);
                      if (have_exception) {
                        KILL(zTransaction)(&z3zE3214);
                        KILL(zTransaction)(&z3zE3215);
                        KILL(zTransaction)(&z3zE3216);
                        KILL(zTransaction)(&z3zE3217);
                        KILL(zTransaction)(&z3zE3218);
                        KILL(zTransaction)(&z3zE3219);
                        KILL(zTransaction)(&z3zE3220);
                        KILL(zTransaction)(&z3zE3221);
                        KILL(zTransaction)(&z3zE3222);
                        goto end_block_exception_3309;
                      }
                    }
                    struct zTransaction z3zE3223;
                    CREATE(zTransaction)(&z3zE3223);
                    {
                      struct zRlpFieldRef zauth_f;
                      zauth_f = z2zE3259.ztup0;
                      struct zRlpCursor z1zE18;
                      z1zE18 = z2zE3259.ztup1;
                      struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3260;
                      {
                        z2zE3260 = zrlp_cursor_pop(z1zE18);
                        if (have_exception) {
                          KILL(zTransaction)(&z3zE3214);
                          KILL(zTransaction)(&z3zE3215);
                          KILL(zTransaction)(&z3zE3216);
                          KILL(zTransaction)(&z3zE3217);
                          KILL(zTransaction)(&z3zE3218);
                          KILL(zTransaction)(&z3zE3219);
                          KILL(zTransaction)(&z3zE3220);
                          KILL(zTransaction)(&z3zE3221);
                          KILL(zTransaction)(&z3zE3222);
                          KILL(zTransaction)(&z3zE3223);
                          goto end_block_exception_3309;
                        }
                      }
                      struct zTransaction z3zE3224;
                      CREATE(zTransaction)(&z3zE3224);
                      {
                        struct zRlpFieldRef zv_f;
                        zv_f = z2zE3260.ztup0;
                        struct zRlpCursor z1zE19;
                        z1zE19 = z2zE3260.ztup1;
                        struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3261;
                        {
                          z2zE3261 = zrlp_cursor_pop(z1zE19);
                          if (have_exception) {
                            KILL(zTransaction)(&z3zE3214);
                            KILL(zTransaction)(&z3zE3215);
                            KILL(zTransaction)(&z3zE3216);
                            KILL(zTransaction)(&z3zE3217);
                            KILL(zTransaction)(&z3zE3218);
                            KILL(zTransaction)(&z3zE3219);
                            KILL(zTransaction)(&z3zE3220);
                            KILL(zTransaction)(&z3zE3221);
                            KILL(zTransaction)(&z3zE3222);
                            KILL(zTransaction)(&z3zE3223);
                            KILL(zTransaction)(&z3zE3224);
                            goto end_block_exception_3309;
                          }
                        }
                        struct zTransaction z3zE3225;
                        CREATE(zTransaction)(&z3zE3225);
                        {
                          struct zRlpFieldRef zr_f;
                          zr_f = z2zE3261.ztup0;
                          struct zRlpCursor z1zE20;
                          z1zE20 = z2zE3261.ztup1;
                          struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzRlpCursorz9 z2zE3262;
                          {
                            z2zE3262 = zrlp_cursor_pop(z1zE20);
                            if (have_exception) {
                              KILL(zTransaction)(&z3zE3214);
                              KILL(zTransaction)(&z3zE3215);
                              KILL(zTransaction)(&z3zE3216);
                              KILL(zTransaction)(&z3zE3217);
                              KILL(zTransaction)(&z3zE3218);
                              KILL(zTransaction)(&z3zE3219);
                              KILL(zTransaction)(&z3zE3220);
                              KILL(zTransaction)(&z3zE3221);
                              KILL(zTransaction)(&z3zE3222);
                              KILL(zTransaction)(&z3zE3223);
                              KILL(zTransaction)(&z3zE3224);
                              KILL(zTransaction)(&z3zE3225);
                              goto end_block_exception_3309;
                            }
                          }
                          struct zTransaction z3zE3226;
                          CREATE(zTransaction)(&z3zE3226);
                          {
                            struct zRlpFieldRef zs_f;
                            zs_f = z2zE3262.ztup0;
                            struct zRlpCursor z1zE21;
                            z1zE21 = z2zE3262.ztup1;
                            unit z3zE3227;
                            {
                              z3zE3227 = zrlp_cursor_expect_end(z1zE21);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE3214);
                                KILL(zTransaction)(&z3zE3215);
                                KILL(zTransaction)(&z3zE3216);
                                KILL(zTransaction)(&z3zE3217);
                                KILL(zTransaction)(&z3zE3218);
                                KILL(zTransaction)(&z3zE3219);
                                KILL(zTransaction)(&z3zE3220);
                                KILL(zTransaction)(&z3zE3221);
                                KILL(zTransaction)(&z3zE3222);
                                KILL(zTransaction)(&z3zE3223);
                                KILL(zTransaction)(&z3zE3224);
                                KILL(zTransaction)(&z3zE3225);
                                KILL(zTransaction)(&z3zE3226);
                                goto end_block_exception_3309;
                              }
                            }
                            sail_u256 zv;
                            {
                              zv = zrlp_ref_word(zv_f);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE3214);
                                KILL(zTransaction)(&z3zE3215);
                                KILL(zTransaction)(&z3zE3216);
                                KILL(zTransaction)(&z3zE3217);
                                KILL(zTransaction)(&z3zE3218);
                                KILL(zTransaction)(&z3zE3219);
                                KILL(zTransaction)(&z3zE3220);
                                KILL(zTransaction)(&z3zE3221);
                                KILL(zTransaction)(&z3zE3222);
                                KILL(zTransaction)(&z3zE3223);
                                KILL(zTransaction)(&z3zE3224);
                                KILL(zTransaction)(&z3zE3225);
                                KILL(zTransaction)(&z3zE3226);
                                goto end_block_exception_3309;
                              }
                            }
                            struct ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9 z2zE3263;
                            CREATE(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3263);
                            {
                              zdecode_access_list(&z2zE3263, zal_f);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE3214);
                                KILL(zTransaction)(&z3zE3215);
                                KILL(zTransaction)(&z3zE3216);
                                KILL(zTransaction)(&z3zE3217);
                                KILL(zTransaction)(&z3zE3218);
                                KILL(zTransaction)(&z3zE3219);
                                KILL(zTransaction)(&z3zE3220);
                                KILL(zTransaction)(&z3zE3221);
                                KILL(zTransaction)(&z3zE3222);
                                KILL(zTransaction)(&z3zE3223);
                                KILL(zTransaction)(&z3zE3224);
                                KILL(zTransaction)(&z3zE3225);
                                KILL(zTransaction)(&z3zE3226);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3263);
                                goto end_block_exception_3309;
                              }
                            }
                            struct zTransaction z3zE3228;
                            CREATE(zTransaction)(&z3zE3228);
                            {
                              zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 zal_addrs;
                              CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                              COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs, z2zE3263.ztup0);
                              zz5listz8z5structz0zzStorageKeyz9 zal_slots;
                              CREATE(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                              COPY(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots, z2zE3263.ztup1);
                              uint64_t zal_addr_count;
                              zal_addr_count = z2zE3263.ztup2;
                              uint64_t zal_slot_count;
                              zal_slot_count = z2zE3263.ztup3;
                              struct ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9 z2zE3264;
                              CREATE(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE3264);
                              {
                                zdecode_auth_list(&z2zE3264, zauth_f);
                                if (have_exception) {
                                  KILL(zTransaction)(&z3zE3214);
                                  KILL(zTransaction)(&z3zE3215);
                                  KILL(zTransaction)(&z3zE3216);
                                  KILL(zTransaction)(&z3zE3217);
                                  KILL(zTransaction)(&z3zE3218);
                                  KILL(zTransaction)(&z3zE3219);
                                  KILL(zTransaction)(&z3zE3220);
                                  KILL(zTransaction)(&z3zE3221);
                                  KILL(zTransaction)(&z3zE3222);
                                  KILL(zTransaction)(&z3zE3223);
                                  KILL(zTransaction)(&z3zE3224);
                                  KILL(zTransaction)(&z3zE3225);
                                  KILL(zTransaction)(&z3zE3226);
                                  KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3263);
                                  KILL(zTransaction)(&z3zE3228);
                                  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                  KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                  KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE3264);
                                  goto end_block_exception_3309;
                                }
                              }
                              struct zTransaction z3zE3229;
                              CREATE(zTransaction)(&z3zE3229);
                              {
                                zz5listz8z5structz0zzAuthorizzzzationz9 zauthorizzations;
                                CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations, z2zE3264.ztup0);
                                uint64_t zauthorizzation_count;
                                zauthorizzation_count = z2zE3264.ztup1;
                                uint64_t z2zE3266;
                                {
                                  uint64_t z2zE3265;
                                  {
                                    z2zE3265 = zrlp_ref_uint64(znonce_f);
                                    if (have_exception) {
                                      KILL(zTransaction)(&z3zE3214);
                                      KILL(zTransaction)(&z3zE3215);
                                      KILL(zTransaction)(&z3zE3216);
                                      KILL(zTransaction)(&z3zE3217);
                                      KILL(zTransaction)(&z3zE3218);
                                      KILL(zTransaction)(&z3zE3219);
                                      KILL(zTransaction)(&z3zE3220);
                                      KILL(zTransaction)(&z3zE3221);
                                      KILL(zTransaction)(&z3zE3222);
                                      KILL(zTransaction)(&z3zE3223);
                                      KILL(zTransaction)(&z3zE3224);
                                      KILL(zTransaction)(&z3zE3225);
                                      KILL(zTransaction)(&z3zE3226);
                                      KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3263);
                                      KILL(zTransaction)(&z3zE3228);
                                      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                      KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                      KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE3264);
                                      KILL(zTransaction)(&z3zE3229);
                                      KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                      goto end_block_exception_3309;
                                    }
                                  }
                                  z2zE3266 = zword_of_account_nonce(z2zE3265);
                                }
                                uint64_t z2zE3267;
                                {
                                  z2zE3267 = zrlp_ref_uint64(zchain_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE3214);
                                    KILL(zTransaction)(&z3zE3215);
                                    KILL(zTransaction)(&z3zE3216);
                                    KILL(zTransaction)(&z3zE3217);
                                    KILL(zTransaction)(&z3zE3218);
                                    KILL(zTransaction)(&z3zE3219);
                                    KILL(zTransaction)(&z3zE3220);
                                    KILL(zTransaction)(&z3zE3221);
                                    KILL(zTransaction)(&z3zE3222);
                                    KILL(zTransaction)(&z3zE3223);
                                    KILL(zTransaction)(&z3zE3224);
                                    KILL(zTransaction)(&z3zE3225);
                                    KILL(zTransaction)(&z3zE3226);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3263);
                                    KILL(zTransaction)(&z3zE3228);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE3264);
                                    KILL(zTransaction)(&z3zE3229);
                                    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                    goto end_block_exception_3309;
                                  }
                                }
                                uint64_t z2zE3268;
                                {
                                  z2zE3268 = zrlp_ref_gas(zgas_f, zfork);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE3214);
                                    KILL(zTransaction)(&z3zE3215);
                                    KILL(zTransaction)(&z3zE3216);
                                    KILL(zTransaction)(&z3zE3217);
                                    KILL(zTransaction)(&z3zE3218);
                                    KILL(zTransaction)(&z3zE3219);
                                    KILL(zTransaction)(&z3zE3220);
                                    KILL(zTransaction)(&z3zE3221);
                                    KILL(zTransaction)(&z3zE3222);
                                    KILL(zTransaction)(&z3zE3223);
                                    KILL(zTransaction)(&z3zE3224);
                                    KILL(zTransaction)(&z3zE3225);
                                    KILL(zTransaction)(&z3zE3226);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3263);
                                    KILL(zTransaction)(&z3zE3228);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE3264);
                                    KILL(zTransaction)(&z3zE3229);
                                    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                    goto end_block_exception_3309;
                                  }
                                }
                                bool z2zE3270;
                                {
                                  uint64_t z2zE3269;
                                  z2zE3269 = zto_f.zcontent_len;
                                  z2zE3270 = (z2zE3269 == UINT64_C(0));
                                }
                                sail_fixed_bytes_20 z2zE3272;
                                {
                                  sail_u256 z2zE3271;
                                  {
                                    z2zE3271 = zrlp_ref_word(zto_f);
                                    if (have_exception) {
                                      KILL(zTransaction)(&z3zE3214);
                                      KILL(zTransaction)(&z3zE3215);
                                      KILL(zTransaction)(&z3zE3216);
                                      KILL(zTransaction)(&z3zE3217);
                                      KILL(zTransaction)(&z3zE3218);
                                      KILL(zTransaction)(&z3zE3219);
                                      KILL(zTransaction)(&z3zE3220);
                                      KILL(zTransaction)(&z3zE3221);
                                      KILL(zTransaction)(&z3zE3222);
                                      KILL(zTransaction)(&z3zE3223);
                                      KILL(zTransaction)(&z3zE3224);
                                      KILL(zTransaction)(&z3zE3225);
                                      KILL(zTransaction)(&z3zE3226);
                                      KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3263);
                                      KILL(zTransaction)(&z3zE3228);
                                      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                      KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                      KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE3264);
                                      KILL(zTransaction)(&z3zE3229);
                                      KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                      goto end_block_exception_3309;
                                    }
                                  }
                                  z2zE3272 = zword_to_address(z2zE3271);
                                }
                                sail_u256 z2zE3273;
                                {
                                  z2zE3273 = zrlp_ref_uint_word(zvalue_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE3214);
                                    KILL(zTransaction)(&z3zE3215);
                                    KILL(zTransaction)(&z3zE3216);
                                    KILL(zTransaction)(&z3zE3217);
                                    KILL(zTransaction)(&z3zE3218);
                                    KILL(zTransaction)(&z3zE3219);
                                    KILL(zTransaction)(&z3zE3220);
                                    KILL(zTransaction)(&z3zE3221);
                                    KILL(zTransaction)(&z3zE3222);
                                    KILL(zTransaction)(&z3zE3223);
                                    KILL(zTransaction)(&z3zE3224);
                                    KILL(zTransaction)(&z3zE3225);
                                    KILL(zTransaction)(&z3zE3226);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3263);
                                    KILL(zTransaction)(&z3zE3228);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE3264);
                                    KILL(zTransaction)(&z3zE3229);
                                    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                    goto end_block_exception_3309;
                                  }
                                }
                                struct zByteSliceFields z2zE3274;
                                {
                                  z2zE3274 = ztx_input_span(zdata_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE3214);
                                    KILL(zTransaction)(&z3zE3215);
                                    KILL(zTransaction)(&z3zE3216);
                                    KILL(zTransaction)(&z3zE3217);
                                    KILL(zTransaction)(&z3zE3218);
                                    KILL(zTransaction)(&z3zE3219);
                                    KILL(zTransaction)(&z3zE3220);
                                    KILL(zTransaction)(&z3zE3221);
                                    KILL(zTransaction)(&z3zE3222);
                                    KILL(zTransaction)(&z3zE3223);
                                    KILL(zTransaction)(&z3zE3224);
                                    KILL(zTransaction)(&z3zE3225);
                                    KILL(zTransaction)(&z3zE3226);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3263);
                                    KILL(zTransaction)(&z3zE3228);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE3264);
                                    KILL(zTransaction)(&z3zE3229);
                                    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                    goto end_block_exception_3309;
                                  }
                                }
                                sail_u256 z2zE3275;
                                {
                                  z2zE3275 = zrlp_ref_uint_word(zmf_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE3214);
                                    KILL(zTransaction)(&z3zE3215);
                                    KILL(zTransaction)(&z3zE3216);
                                    KILL(zTransaction)(&z3zE3217);
                                    KILL(zTransaction)(&z3zE3218);
                                    KILL(zTransaction)(&z3zE3219);
                                    KILL(zTransaction)(&z3zE3220);
                                    KILL(zTransaction)(&z3zE3221);
                                    KILL(zTransaction)(&z3zE3222);
                                    KILL(zTransaction)(&z3zE3223);
                                    KILL(zTransaction)(&z3zE3224);
                                    KILL(zTransaction)(&z3zE3225);
                                    KILL(zTransaction)(&z3zE3226);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3263);
                                    KILL(zTransaction)(&z3zE3228);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE3264);
                                    KILL(zTransaction)(&z3zE3229);
                                    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                    goto end_block_exception_3309;
                                  }
                                }
                                sail_u256 z2zE3276;
                                {
                                  z2zE3276 = zrlp_ref_uint_word(zmp_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE3214);
                                    KILL(zTransaction)(&z3zE3215);
                                    KILL(zTransaction)(&z3zE3216);
                                    KILL(zTransaction)(&z3zE3217);
                                    KILL(zTransaction)(&z3zE3218);
                                    KILL(zTransaction)(&z3zE3219);
                                    KILL(zTransaction)(&z3zE3220);
                                    KILL(zTransaction)(&z3zE3221);
                                    KILL(zTransaction)(&z3zE3222);
                                    KILL(zTransaction)(&z3zE3223);
                                    KILL(zTransaction)(&z3zE3224);
                                    KILL(zTransaction)(&z3zE3225);
                                    KILL(zTransaction)(&z3zE3226);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3263);
                                    KILL(zTransaction)(&z3zE3228);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE3264);
                                    KILL(zTransaction)(&z3zE3229);
                                    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                    goto end_block_exception_3309;
                                  }
                                }
                                sail_fixed_bytes_32 z2zE3278;
                                {
                                  struct zByteSliceFields z2zE3277;
                                  {
                                    z2zE3277 = ztx_sig_span(zchain_f, zv_f);
                                    if (have_exception) {
                                      KILL(zTransaction)(&z3zE3214);
                                      KILL(zTransaction)(&z3zE3215);
                                      KILL(zTransaction)(&z3zE3216);
                                      KILL(zTransaction)(&z3zE3217);
                                      KILL(zTransaction)(&z3zE3218);
                                      KILL(zTransaction)(&z3zE3219);
                                      KILL(zTransaction)(&z3zE3220);
                                      KILL(zTransaction)(&z3zE3221);
                                      KILL(zTransaction)(&z3zE3222);
                                      KILL(zTransaction)(&z3zE3223);
                                      KILL(zTransaction)(&z3zE3224);
                                      KILL(zTransaction)(&z3zE3225);
                                      KILL(zTransaction)(&z3zE3226);
                                      KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3263);
                                      KILL(zTransaction)(&z3zE3228);
                                      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                      KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                      KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE3264);
                                      KILL(zTransaction)(&z3zE3229);
                                      KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                      goto end_block_exception_3309;
                                    }
                                  }
                                  {
                                    z2zE3278 = ztx_signing_hash(zSetCodeTx, z2zE3277, zv);
                                    if (have_exception) {
                                      KILL(zTransaction)(&z3zE3214);
                                      KILL(zTransaction)(&z3zE3215);
                                      KILL(zTransaction)(&z3zE3216);
                                      KILL(zTransaction)(&z3zE3217);
                                      KILL(zTransaction)(&z3zE3218);
                                      KILL(zTransaction)(&z3zE3219);
                                      KILL(zTransaction)(&z3zE3220);
                                      KILL(zTransaction)(&z3zE3221);
                                      KILL(zTransaction)(&z3zE3222);
                                      KILL(zTransaction)(&z3zE3223);
                                      KILL(zTransaction)(&z3zE3224);
                                      KILL(zTransaction)(&z3zE3225);
                                      KILL(zTransaction)(&z3zE3226);
                                      KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3263);
                                      KILL(zTransaction)(&z3zE3228);
                                      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                      KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                      KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE3264);
                                      KILL(zTransaction)(&z3zE3229);
                                      KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                      goto end_block_exception_3309;
                                    }
                                  }
                                }
                                sail_u256 z2zE3279;
                                {
                                  z2zE3279 = zrlp_ref_uint_word(zr_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE3214);
                                    KILL(zTransaction)(&z3zE3215);
                                    KILL(zTransaction)(&z3zE3216);
                                    KILL(zTransaction)(&z3zE3217);
                                    KILL(zTransaction)(&z3zE3218);
                                    KILL(zTransaction)(&z3zE3219);
                                    KILL(zTransaction)(&z3zE3220);
                                    KILL(zTransaction)(&z3zE3221);
                                    KILL(zTransaction)(&z3zE3222);
                                    KILL(zTransaction)(&z3zE3223);
                                    KILL(zTransaction)(&z3zE3224);
                                    KILL(zTransaction)(&z3zE3225);
                                    KILL(zTransaction)(&z3zE3226);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3263);
                                    KILL(zTransaction)(&z3zE3228);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE3264);
                                    KILL(zTransaction)(&z3zE3229);
                                    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                    goto end_block_exception_3309;
                                  }
                                }
                                sail_u256 z2zE3280;
                                {
                                  z2zE3280 = zrlp_ref_uint_word(zs_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE3214);
                                    KILL(zTransaction)(&z3zE3215);
                                    KILL(zTransaction)(&z3zE3216);
                                    KILL(zTransaction)(&z3zE3217);
                                    KILL(zTransaction)(&z3zE3218);
                                    KILL(zTransaction)(&z3zE3219);
                                    KILL(zTransaction)(&z3zE3220);
                                    KILL(zTransaction)(&z3zE3221);
                                    KILL(zTransaction)(&z3zE3222);
                                    KILL(zTransaction)(&z3zE3223);
                                    KILL(zTransaction)(&z3zE3224);
                                    KILL(zTransaction)(&z3zE3225);
                                    KILL(zTransaction)(&z3zE3226);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3263);
                                    KILL(zTransaction)(&z3zE3228);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE3264);
                                    KILL(zTransaction)(&z3zE3229);
                                    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                    goto end_block_exception_3309;
                                  }
                                }
                                z3zE3229.zaccess_list_address_count = zal_addr_count;
                                COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE3229)->zaccess_list_addresses), zal_addrs);
                                z3zE3229.zaccess_list_slot_count = zal_slot_count;
                                COPY(zz5listz8z5structz0zzStorageKeyz9)(&((&z3zE3229)->zaccess_list_slots), zal_slots);
                                z3zE3229.zauthorizzation_count = zauthorizzation_count;
                                COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&((&z3zE3229)->zauthorizzations), zauthorizzations);
                                z3zE3229.zblob_hashes = zEMPTY_BLOB_HASHES;
                                z3zE3229.zchain_id = z2zE3267;
                                z3zE3229.zgas_limit = z2zE3268;
                                z3zE3229.zinput_src = z2zE3274;
                                z3zE3229.zis_create = z2zE3270;
                                z3zE3229.zmax_blob_fee = zZERO_WORD;
                                z3zE3229.zmax_fee = z2zE3275;
                                z3zE3229.zmax_priority_fee = z2zE3276;
                                z3zE3229.znonce = u256_of_fbits(z2zE3266);
                                z3zE3229.zpubkey = zpubkey;
                                z3zE3229.zraw = ztx;
                                z3zE3229.zrecipient = z2zE3272;
                                z3zE3229.zsender = zsender;
                                z3zE3229.zsig_r = z2zE3279;
                                z3zE3229.zsig_s = z2zE3280;
                                z3zE3229.zsig_v = zv;
                                z3zE3229.zsigning_hash = z2zE3278;
                                z3zE3229.ztx_type = zSetCodeTx;
                                z3zE3229.zvalue = z2zE3273;
                                KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                goto finish_match_3306;
                              }
                            case_3307: ;
                              sail_match_failure("decode_set_code_tx");
                            finish_match_3306: ;
                              COPY(zTransaction)(&z3zE3228, z3zE3229);
                              KILL(zTransaction)(&z3zE3229);
                              KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE3264);
                              KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                              KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                              goto finish_match_3304;
                            }
                          case_3305: ;
                            sail_match_failure("decode_set_code_tx");
                          finish_match_3304: ;
                            COPY(zTransaction)(&z3zE3226, z3zE3228);
                            KILL(zTransaction)(&z3zE3228);
                            KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3263);
                            goto finish_match_3302;
                          }
                        case_3303: ;
                          sail_match_failure("decode_set_code_tx");
                        finish_match_3302: ;
                          COPY(zTransaction)(&z3zE3225, z3zE3226);
                          KILL(zTransaction)(&z3zE3226);
                          goto finish_match_3300;
                        }
                      case_3301: ;
                        sail_match_failure("decode_set_code_tx");
                      finish_match_3300: ;
                        COPY(zTransaction)(&z3zE3224, z3zE3225);
                        KILL(zTransaction)(&z3zE3225);
                        goto finish_match_3298;
                      }
                    case_3299: ;
                      sail_match_failure("decode_set_code_tx");
                    finish_match_3298: ;
                      COPY(zTransaction)(&z3zE3223, z3zE3224);
                      KILL(zTransaction)(&z3zE3224);
                      goto finish_match_3296;
                    }
                  case_3297: ;
                    sail_match_failure("decode_set_code_tx");
                  finish_match_3296: ;
                    COPY(zTransaction)(&z3zE3222, z3zE3223);
                    KILL(zTransaction)(&z3zE3223);
                    goto finish_match_3294;
                  }
                case_3295: ;
                  sail_match_failure("decode_set_code_tx");
                finish_match_3294: ;
                  COPY(zTransaction)(&z3zE3221, z3zE3222);
                  KILL(zTransaction)(&z3zE3222);
                  goto finish_match_3292;
                }
              case_3293: ;
                sail_match_failure("decode_set_code_tx");
              finish_match_3292: ;
                COPY(zTransaction)(&z3zE3220, z3zE3221);
                KILL(zTransaction)(&z3zE3221);
                goto finish_match_3290;
              }
            case_3291: ;
              sail_match_failure("decode_set_code_tx");
            finish_match_3290: ;
              COPY(zTransaction)(&z3zE3219, z3zE3220);
              KILL(zTransaction)(&z3zE3220);
              goto finish_match_3288;
            }
          case_3289: ;
            sail_match_failure("decode_set_code_tx");
          finish_match_3288: ;
            COPY(zTransaction)(&z3zE3218, z3zE3219);
            KILL(zTransaction)(&z3zE3219);
            goto finish_match_3286;
          }
        case_3287: ;
          sail_match_failure("decode_set_code_tx");
        finish_match_3286: ;
          COPY(zTransaction)(&z3zE3217, z3zE3218);
          KILL(zTransaction)(&z3zE3218);
          goto finish_match_3284;
        }
      case_3285: ;
        sail_match_failure("decode_set_code_tx");
      finish_match_3284: ;
        COPY(zTransaction)(&z3zE3216, z3zE3217);
        KILL(zTransaction)(&z3zE3217);
        goto finish_match_3282;
      }
    case_3283: ;
      sail_match_failure("decode_set_code_tx");
    finish_match_3282: ;
      COPY(zTransaction)(&z3zE3215, z3zE3216);
      KILL(zTransaction)(&z3zE3216);
      goto finish_match_3280;
    }
  case_3281: ;
    sail_match_failure("decode_set_code_tx");
  finish_match_3280: ;
    COPY(zTransaction)(&z3zE3214, z3zE3215);
    KILL(zTransaction)(&z3zE3215);
    goto finish_match_3278;
  }
case_3279: ;
  sail_match_failure("decode_set_code_tx");
finish_match_3278: ;
  COPY(zTransaction)((*(&z8zE203)), z3zE3214);
  KILL(zTransaction)(&z3zE3214);
end_function_3308: ;
  goto end_function_4062;
end_block_exception_3309: ;
  goto end_function_4062;
end_function_4062: ;
}

void zrlp_decode_tx(struct zTransaction *z8zE204, struct zByteSliceFields ztx, struct zByteSliceFields zpubkey, enum zFork zfork)
{
  sail_fixed_bytes_20 zsender;
  {
    sail_u256 z2zE3249;
    {
      sail_fixed_bytes_32 z2zE3248;
      {
        struct zByteSliceFields z2zE3247;
        z2zE3247 = zsub_slice(zpubkey, UINT64_C(1), zPUBLIC_KEY_BODY_LENGTH);
        z2zE3248 = zkeccak256_slice(z2zE3247);
      }
      z2zE3249 = zhash_to_word(z2zE3248);
    }
    zsender = zword_to_address(z2zE3249);
  }
  uint64_t ztx_length;
  ztx_length = ztx.zlen;
  uint64_t zb0;
  {
    bool z2zE3245;
    z2zE3245 = (ztx_length == UINT64_C(0));
    if (z2zE3245) {
      struct zexception z2zE3246;
      CREATE(zexception)(&z2zE3246);
      zInvalidBlock(&z2zE3246, zRlpDecode);
      COPY(zexception)(current_exception, z2zE3246);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/lib/rlp/tx.sail:636.12-636.41");
      KILL(zexception)(&z2zE3246);
      goto end_block_exception_3277;
      /* unreachable after throw */
      KILL(zexception)(&z2zE3246);
    } else {  zb0 = zslice_byte(ztx, UINT64_C(0));  }
  }
  uint64_t zttype;
  {
    bool z2zE3244;
    {
      uint64_t z2zE3243;
      z2zE3243 = (safe_rshift(UINT64_MAX, 64 - 2) & (zb0 >> UINT64_C(6)));
      z2zE3244 = (z2zE3243 == UINT64_C(0b11));
    }
    if (z2zE3244) {  zttype = UINT64_C(0x00);  } else {  zttype = zb0;  }
  }
  bool ztyped;
  ztyped = (zttype != UINT64_C(0x00));
  struct zByteSliceFields zpayload;
  if (ztyped) {
    bool z2zE3240;
    z2zE3240 = (!(ztx_length < UINT64_C(1)));
    if (z2zE3240) {
      uint64_t z2zE3241;
      {    z2zE3241 = (ztx_length - UINT64_C(1));
      }
      zpayload = zsub_slice(ztx, UINT64_C(1), z2zE3241);
    } else {
      struct zexception z2zE3242;
      CREATE(zexception)(&z2zE3242);
      zInvalidBlock(&z2zE3242, zRlpDecode);
      COPY(zexception)(current_exception, z2zE3242);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/lib/rlp/tx.sail:646.16-646.45");
      KILL(zexception)(&z2zE3242);
      goto end_block_exception_3277;
      /* unreachable after throw */
      KILL(zexception)(&z2zE3242);
    }
  } else {  zpayload = ztx;  }
  struct zRlpCursor zfields;
  {
    zfields = zrlp_node_cursor(zpayload);
    if (have_exception) {  goto end_block_exception_3277;  }
  }
  struct zTransaction z3zE3203;
  CREATE(zTransaction)(&z3zE3203);
  {
    uint64_t zp0z3;
    zp0z3 = zttype;
    bool z3zE3209;
    z3zE3209 = (zp0z3 == UINT64_C(0x00));
    if (!(z3zE3209)) {  goto case_3275;  }
    {
      zdecode_legacy_tx(&z3zE3203, ztx, zpubkey, zfork, zsender, zfields);
      if (have_exception) {
        KILL(zTransaction)(&z3zE3203);
        goto end_block_exception_3277;
      }
    }
    goto finish_match_3269;
  }
case_3275: ;
  {
    uint64_t z3zE3210;
    z3zE3210 = zttype;
    bool z3zE3208;
    z3zE3208 = (z3zE3210 == UINT64_C(0x01));
    if (!(z3zE3208)) {  goto case_3274;  }
    {
      zdecode_access_list_tx(&z3zE3203, ztx, zpubkey, zfork, zsender, zfields);
      if (have_exception) {
        KILL(zTransaction)(&z3zE3203);
        goto end_block_exception_3277;
      }
    }
    goto finish_match_3269;
  }
case_3274: ;
  {
    uint64_t z3zE3211;
    z3zE3211 = zttype;
    bool z3zE3207;
    z3zE3207 = (z3zE3211 == UINT64_C(0x02));
    if (!(z3zE3207)) {  goto case_3273;  }
    {
      zdecode_fee_market_tx(&z3zE3203, ztx, zpubkey, zfork, zsender, zfields);
      if (have_exception) {
        KILL(zTransaction)(&z3zE3203);
        goto end_block_exception_3277;
      }
    }
    goto finish_match_3269;
  }
case_3273: ;
  {
    uint64_t z3zE3212;
    z3zE3212 = zttype;
    bool z3zE3206;
    z3zE3206 = (z3zE3212 == UINT64_C(0x03));
    if (!(z3zE3206)) {  goto case_3272;  }
    {
      zdecode_blob_tx(&z3zE3203, ztx, zpubkey, zfork, zsender, zfields);
      if (have_exception) {
        KILL(zTransaction)(&z3zE3203);
        goto end_block_exception_3277;
      }
    }
    goto finish_match_3269;
  }
case_3272: ;
  {
    uint64_t z3zE3213;
    z3zE3213 = zttype;
    bool z3zE3205;
    z3zE3205 = (z3zE3213 == UINT64_C(0x04));
    if (!(z3zE3205)) {  goto case_3271;  }
    {
      zdecode_set_code_tx(&z3zE3203, ztx, zpubkey, zfork, zsender, zfields);
      if (have_exception) {
        KILL(zTransaction)(&z3zE3203);
        goto end_block_exception_3277;
      }
    }
    goto finish_match_3269;
  }
case_3271: ;
  {
    struct zexception z2zE3239;
    CREATE(zexception)(&z2zE3239);
    zInvalidBlock(&z2zE3239, zRlpDecode);
    COPY(zexception)(current_exception, z2zE3239);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/tx.sail:659.13-659.42");
    KILL(zTransaction)(&z3zE3203);
    KILL(zexception)(&z2zE3239);
    goto end_block_exception_3277;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3239);
    goto finish_match_3269;
  }
case_3270: ;
finish_match_3269: ;
  COPY(zTransaction)((*(&z8zE204)), z3zE3203);
  KILL(zTransaction)(&z3zE3203);
end_function_3276: ;
  goto end_function_4061;
end_block_exception_3277: ;
  goto end_function_4061;
end_function_4061: ;
}

