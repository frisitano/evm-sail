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
let_end_303: ;
}
void kill_letbind_92(void) {    KILL(zAccessListDecode)(&zEMPTY_ACCESS_LIST_DECODE);
}

void zdecode_access_list_keys(struct zAccessListDecode *z8zE198, struct zRlpCursorFields zcursor, sail_fixed_bytes_20 zaddr, struct zAccessListDecode ztail)
{
  bool z2zE3492;
  z2zE3492 = zrlp_cursor_empty(zcursor);
  if (z2zE3492) {
    zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE3493;
    CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE3493);
    COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE3493, ztail.zaddresses);
    zz5listz8z5structz0zzStorageKeyz9 z2zE3494;
    CREATE(zz5listz8z5structz0zzStorageKeyz9)(&z2zE3494);
    COPY(zz5listz8z5structz0zzStorageKeyz9)(&z2zE3494, ztail.zstorage_slots);
    uint64_t z2zE3495;
    z2zE3495 = ztail.zaddress_count;
    uint64_t z2zE3496;
    z2zE3496 = ztail.zslot_count;
    struct zAccessListDecode z3zE3474;
    CREATE(zAccessListDecode)(&z3zE3474);
    z3zE3474.zaddress_count = z2zE3495;
    COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE3474)->zaddresses), z2zE3493);
    z3zE3474.zslot_count = z2zE3496;
    COPY(zz5listz8z5structz0zzStorageKeyz9)(&((&z3zE3474)->zstorage_slots), z2zE3494);
    COPY(zAccessListDecode)((*(&z8zE198)), z3zE3474);
    KILL(zAccessListDecode)(&z3zE3474);
    KILL(zz5listz8z5structz0zzStorageKeyz9)(&z2zE3494);
    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE3493);
  } else {
    struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3497;
    {
      z2zE3497 = zrlp_cursor_pop(zcursor);
      if (have_exception) {  goto end_block_exception_3456;  }
    }
    struct zAccessListDecode z3zE3470;
    CREATE(zAccessListDecode)(&z3zE3470);
    {
      struct zRlpFieldRefFields zkey;
      zkey = z2zE3497.ztup0;
      struct zRlpCursorFields znext;
      znext = z2zE3497.ztup1;
      struct zStorageKey zstorage_key;
      {
        sail_u256 z2zE3504;
        {
          z2zE3504 = zrlp_ref_word(zkey);
          if (have_exception) {
            KILL(zAccessListDecode)(&z3zE3470);
            goto end_block_exception_3456;
          }
        }
        struct zStorageKey z3zE3471;
        z3zE3471.zaddr = zaddr;
        z3zE3471.zslot = z2zE3504;
        zstorage_key = z3zE3471;
      }
      struct zAccessListDecode zresult;
      CREATE(zAccessListDecode)(&zresult);
      {
        zdecode_access_list_keys(&zresult, znext, zaddr, ztail);
        if (have_exception) {
          KILL(zAccessListDecode)(&z3zE3470);
          KILL(zAccessListDecode)(&zresult);
          goto end_block_exception_3456;
        }
      }
      zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE3498;
      CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE3498);
      COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE3498, zresult.zaddresses);
      zz5listz8z5structz0zzStorageKeyz9 z2zE3500;
      CREATE(zz5listz8z5structz0zzStorageKeyz9)(&z2zE3500);
      {
        zz5listz8z5structz0zzStorageKeyz9 z2zE3499;
        CREATE(zz5listz8z5structz0zzStorageKeyz9)(&z2zE3499);
        COPY(zz5listz8z5structz0zzStorageKeyz9)(&z2zE3499, zresult.zstorage_slots);
        zconsz3z5structz0zzStorageKey(&z2zE3500, zstorage_key, z2zE3499);
        KILL(zz5listz8z5structz0zzStorageKeyz9)(&z2zE3499);
      }
      uint64_t z2zE3501;
      z2zE3501 = zresult.zaddress_count;
      uint64_t z2zE3503;
      {
        uint64_t z2zE3502;
        z2zE3502 = zresult.zslot_count;
        z2zE3503 = zrlp_item_count_incrementzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE3502);
      }
      struct zAccessListDecode z3zE3472;
      CREATE(zAccessListDecode)(&z3zE3472);
      z3zE3472.zaddress_count = z2zE3501;
      COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE3472)->zaddresses), z2zE3498);
      z3zE3472.zslot_count = z2zE3503;
      COPY(zz5listz8z5structz0zzStorageKeyz9)(&((&z3zE3472)->zstorage_slots), z2zE3500);
      COPY(zAccessListDecode)(&z3zE3470, z3zE3472);
      KILL(zAccessListDecode)(&z3zE3472);
      KILL(zz5listz8z5structz0zzStorageKeyz9)(&z2zE3500);
      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE3498);
      KILL(zAccessListDecode)(&zresult);
      goto finish_match_3453;
    }
  case_3454: ;
    sail_match_failure("decode_access_list_keys");
  finish_match_3453: ;
    COPY(zAccessListDecode)((*(&z8zE198)), z3zE3470);
    KILL(zAccessListDecode)(&z3zE3470);
  }
end_function_3455: ;
  goto end_function_4083;
end_block_exception_3456: ;
  goto end_function_4083;
end_function_4083: ;
}

void zdecode_access_list_entries(struct zAccessListDecode *z8zE199, struct zRlpCursorFields zcursor)
{
  bool z2zE3479;
  z2zE3479 = zrlp_cursor_empty(zcursor);
  if (z2zE3479) {
    struct zAccessListDecode z3zE3467;
    CREATE(zAccessListDecode)(&z3zE3467);
    z3zE3467.zaddress_count = UINT64_C(0);
    zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z3zE3468;
    CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3468);
    COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE3467)->zaddresses), z3zE3468);
    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3468);
    z3zE3467.zslot_count = UINT64_C(0);
    zz5listz8z5structz0zzStorageKeyz9 z3zE3469;
    CREATE(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3469);
    COPY(zz5listz8z5structz0zzStorageKeyz9)(&((&z3zE3467)->zstorage_slots), z3zE3469);
    KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3469);
    COPY(zAccessListDecode)((*(&z8zE199)), z3zE3467);
    KILL(zAccessListDecode)(&z3zE3467);
  } else {
    struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3480;
    {
      z2zE3480 = zrlp_cursor_pop(zcursor);
      if (have_exception) {  goto end_block_exception_3452;  }
    }
    struct zAccessListDecode z3zE3459;
    CREATE(zAccessListDecode)(&z3zE3459);
    {
      struct zRlpFieldRefFields zentry;
      zentry = z2zE3480.ztup0;
      struct zRlpCursorFields znext;
      znext = z2zE3480.ztup1;
      struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3482;
      {
        struct zRlpCursorFields z2zE3481;
        {
          z2zE3481 = zrlp_ref_cursor(zentry);
          if (have_exception) {
            KILL(zAccessListDecode)(&z3zE3459);
            goto end_block_exception_3452;
          }
        }
        {
          z2zE3482 = zrlp_cursor_pop(z2zE3481);
          if (have_exception) {
            KILL(zAccessListDecode)(&z3zE3459);
            goto end_block_exception_3452;
          }
        }
      }
      struct zAccessListDecode z3zE3460;
      CREATE(zAccessListDecode)(&z3zE3460);
      {
        struct zRlpFieldRefFields zaddr_f;
        zaddr_f = z2zE3482.ztup0;
        struct zRlpCursorFields zentry_fields;
        zentry_fields = z2zE3482.ztup1;
        struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3483;
        {
          z2zE3483 = zrlp_cursor_pop(zentry_fields);
          if (have_exception) {
            KILL(zAccessListDecode)(&z3zE3459);
            KILL(zAccessListDecode)(&z3zE3460);
            goto end_block_exception_3452;
          }
        }
        struct zAccessListDecode z3zE3461;
        CREATE(zAccessListDecode)(&z3zE3461);
        {
          struct zRlpFieldRefFields zkeys_f;
          zkeys_f = z2zE3483.ztup0;
          struct zRlpCursorFields z1zE73;
          z1zE73 = z2zE3483.ztup1;
          unit z3zE3462;
          {
            z3zE3462 = zrlp_cursor_expect_end(z1zE73);
            if (have_exception) {
              KILL(zAccessListDecode)(&z3zE3459);
              KILL(zAccessListDecode)(&z3zE3460);
              KILL(zAccessListDecode)(&z3zE3461);
              goto end_block_exception_3452;
            }
          }
          sail_fixed_bytes_20 zaddr;
          {
            sail_u256 z2zE3491;
            {
              z2zE3491 = zrlp_ref_word(zaddr_f);
              if (have_exception) {
                KILL(zAccessListDecode)(&z3zE3459);
                KILL(zAccessListDecode)(&z3zE3460);
                KILL(zAccessListDecode)(&z3zE3461);
                goto end_block_exception_3452;
              }
            }
            zaddr = zword_to_address(z2zE3491);
          }
          struct zAccessListDecode ztail;
          CREATE(zAccessListDecode)(&ztail);
          {
            zdecode_access_list_entries(&ztail, znext);
            if (have_exception) {
              KILL(zAccessListDecode)(&z3zE3459);
              KILL(zAccessListDecode)(&z3zE3460);
              KILL(zAccessListDecode)(&z3zE3461);
              KILL(zAccessListDecode)(&ztail);
              goto end_block_exception_3452;
            }
          }
          struct zAccessListDecode zresult;
          CREATE(zAccessListDecode)(&zresult);
          {
            struct zRlpCursorFields z2zE3490;
            {
              z2zE3490 = zrlp_ref_cursor(zkeys_f);
              if (have_exception) {
                KILL(zAccessListDecode)(&z3zE3459);
                KILL(zAccessListDecode)(&z3zE3460);
                KILL(zAccessListDecode)(&z3zE3461);
                KILL(zAccessListDecode)(&ztail);
                KILL(zAccessListDecode)(&zresult);
                goto end_block_exception_3452;
              }
            }
            {
              zdecode_access_list_keys(&zresult, z2zE3490, zaddr, ztail);
              if (have_exception) {
                KILL(zAccessListDecode)(&z3zE3459);
                KILL(zAccessListDecode)(&z3zE3460);
                KILL(zAccessListDecode)(&z3zE3461);
                KILL(zAccessListDecode)(&ztail);
                KILL(zAccessListDecode)(&zresult);
                goto end_block_exception_3452;
              }
            }
          }
          zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE3485;
          CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE3485);
          {
            zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE3484;
            CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE3484);
            COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE3484, zresult.zaddresses);
            zconsz3z5structz0zz__sail_c_repr_fixed_bytesz820z9(&z2zE3485, zaddr, z2zE3484);
            KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE3484);
          }
          zz5listz8z5structz0zzStorageKeyz9 z2zE3486;
          CREATE(zz5listz8z5structz0zzStorageKeyz9)(&z2zE3486);
          COPY(zz5listz8z5structz0zzStorageKeyz9)(&z2zE3486, zresult.zstorage_slots);
          uint64_t z2zE3488;
          {
            uint64_t z2zE3487;
            z2zE3487 = zresult.zaddress_count;
            z2zE3488 = zrlp_item_count_incrementzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE3487);
          }
          uint64_t z2zE3489;
          z2zE3489 = zresult.zslot_count;
          struct zAccessListDecode z3zE3463;
          CREATE(zAccessListDecode)(&z3zE3463);
          z3zE3463.zaddress_count = z2zE3488;
          COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE3463)->zaddresses), z2zE3485);
          z3zE3463.zslot_count = z2zE3489;
          COPY(zz5listz8z5structz0zzStorageKeyz9)(&((&z3zE3463)->zstorage_slots), z2zE3486);
          COPY(zAccessListDecode)(&z3zE3461, z3zE3463);
          KILL(zAccessListDecode)(&z3zE3463);
          KILL(zz5listz8z5structz0zzStorageKeyz9)(&z2zE3486);
          KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE3485);
          KILL(zAccessListDecode)(&zresult);
          KILL(zAccessListDecode)(&ztail);
          goto finish_match_3449;
        }
      case_3450: ;
        sail_match_failure("decode_access_list_entries");
      finish_match_3449: ;
        COPY(zAccessListDecode)(&z3zE3460, z3zE3461);
        KILL(zAccessListDecode)(&z3zE3461);
        goto finish_match_3447;
      }
    case_3448: ;
      sail_match_failure("decode_access_list_entries");
    finish_match_3447: ;
      COPY(zAccessListDecode)(&z3zE3459, z3zE3460);
      KILL(zAccessListDecode)(&z3zE3460);
      goto finish_match_3445;
    }
  case_3446: ;
    sail_match_failure("decode_access_list_entries");
  finish_match_3445: ;
    COPY(zAccessListDecode)((*(&z8zE199)), z3zE3459);
    KILL(zAccessListDecode)(&z3zE3459);
  }
end_function_3451: ;
  goto end_function_4082;
end_block_exception_3452: ;
  goto end_function_4082;
end_function_4082: ;
}

void zdecode_access_list(struct ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9 *z8zE200, struct zRlpFieldRefFields zf)
{
  bool z2zE3472;
  {
    uint64_t z2zE3471;
    {
      struct zByteSliceFields z2zE3470;
      z2zE3470 = zf.zsource;
      z2zE3471 = z2zE3470.zlen;
    }
    z2zE3472 = (!(UINT64_C(1073741824) < z2zE3471));
  }
  if (z2zE3472) {
    struct zAccessListDecode zdecoded;
    CREATE(zAccessListDecode)(&zdecoded);
    {
      struct zRlpCursorFields z2zE3477;
      {
        z2zE3477 = zrlp_ref_cursor(zf);
        if (have_exception) {
          KILL(zAccessListDecode)(&zdecoded);
          goto end_block_exception_3444;
        }
      }
      {
        zdecode_access_list_entries(&zdecoded, z2zE3477);
        if (have_exception) {
          KILL(zAccessListDecode)(&zdecoded);
          goto end_block_exception_3444;
        }
      }
    }
    zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE3473;
    CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE3473);
    COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE3473, zdecoded.zaddresses);
    zz5listz8z5structz0zzStorageKeyz9 z2zE3474;
    CREATE(zz5listz8z5structz0zzStorageKeyz9)(&z2zE3474);
    COPY(zz5listz8z5structz0zzStorageKeyz9)(&z2zE3474, zdecoded.zstorage_slots);
    uint64_t z2zE3475;
    z2zE3475 = zdecoded.zaddress_count;
    uint64_t z2zE3476;
    z2zE3476 = zdecoded.zslot_count;
    struct ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9 z3zE3458;
    CREATE(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z3zE3458);
    COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE3458)->ztup0), z2zE3473);
    COPY(zz5listz8z5structz0zzStorageKeyz9)(&((&z3zE3458)->ztup1), z2zE3474);
    z3zE3458.ztup2 = z2zE3475;
    z3zE3458.ztup3 = z2zE3476;
    COPY(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)((*(&z8zE200)), z3zE3458);
    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z3zE3458);
    KILL(zz5listz8z5structz0zzStorageKeyz9)(&z2zE3474);
    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE3473);
    KILL(zAccessListDecode)(&zdecoded);
  } else {
    struct zexception z2zE3478;
    CREATE(zexception)(&z2zE3478);
    zInvalidBlock(&z2zE3478, zRlpDecode);
    COPY(zexception)(current_exception, z2zE3478);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/tx.sail:126.8-126.37");
    KILL(zexception)(&z2zE3478);
    goto end_block_exception_3444;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3478);
  }
end_function_3443: ;
  goto end_function_4081;
end_block_exception_3444: ;
  goto end_function_4081;
end_function_4081: ;
}

void create_letbind_93(void) {

  uint64_t z3zE121;
  z3zE121 = UINT64_C(33);
  zBLOB_HASH_RLP_LENGTH = z3zE121;
let_end_308: ;
}
void kill_letbind_93(void) {
}

void create_letbind_94(void) {

  uint64_t z3zE122;
  z3zE122 = zWORD_BYTE_LENGTH;
  zBLOB_HASH_LENGTH = z3zE122;
let_end_309: ;
}
void kill_letbind_94(void) {
}

uint64_t zdecode_blob_hash_items(struct zRlpCursorFields zcursor, uint64_t zcount)
{
  uint64_t z8zE201;
  bool z2zE3455;
  z2zE3455 = zrlp_cursor_empty(zcursor);
  if (z2zE3455) {  z8zE201 = zcount;  } else {
    struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3456;
    {
      z2zE3456 = zrlp_cursor_pop(zcursor);
      if (have_exception) {  goto end_block_exception_3442;  }
    }
    uint64_t z3zE3452;
    {
      struct zRlpFieldRefFields zitem;
      zitem = z2zE3456.ztup0;
      struct zRlpCursorFields znext;
      znext = z2zE3456.ztup1;
      bool z2zE3465;
      {
        bool z2zE3464;
        z2zE3464 = zitem.zis_list;
        bool z3zE3455;
        if (z2zE3464) {  z3zE3455 = true;  } else {
          bool z2zE3463;
          {
            uint64_t z2zE3457;
            z2zE3457 = zitem.zfull_len;
            z2zE3463 = (z2zE3457 != zBLOB_HASH_RLP_LENGTH);
          }
          bool z3zE3454;
          if (z2zE3463) {  z3zE3454 = true;  } else {
            bool z2zE3462;
            {
              uint64_t z2zE3458;
              z2zE3458 = zitem.zcontent_len;
              z2zE3462 = (z2zE3458 != zBLOB_HASH_LENGTH);
            }
            bool z3zE3453;
            if (z2zE3462) {  z3zE3453 = true;  } else {
              uint64_t z2zE3461;
              {
                struct zByteSliceFields z2zE3459;
                z2zE3459 = zitem.zsource;
                uint64_t z2zE3460;
                z2zE3460 = zitem.zfull_off;
                z2zE3461 = zslice_byte(z2zE3459, z2zE3460);
              }
              z3zE3453 = (z2zE3461 != UINT64_C(0xA0));
            }
            z3zE3454 = z3zE3453;
          }
          z3zE3455 = z3zE3454;
        }
        z2zE3465 = z3zE3455;
      }
      unit z3zE3456;
      if (z2zE3465) {
        struct zexception z2zE3466;
        CREATE(zexception)(&z2zE3466);
        zInvalidBlock(&z2zE3466, zRlpDecode);
        COPY(zexception)(current_exception, z2zE3466);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/lib/rlp/tx.sail:156.12-156.41");
        KILL(zexception)(&z2zE3466);
        goto end_block_exception_3442;
        /* unreachable after throw */
        KILL(zexception)(&z2zE3466);
      } else {  z3zE3456 = UNIT;  }
      bool z2zE3467;
      z2zE3467 = (zcount < UINT64_C(9));
      if (z2zE3467) {
        uint64_t z2zE3468;
        {    z2zE3468 = (zcount + UINT64_C(1));
        }
        {
          z3zE3452 = zdecode_blob_hash_items(znext, z2zE3468);
          if (have_exception) {  goto end_block_exception_3442;  }
        }
      } else {
        struct zexception z2zE3469;
        CREATE(zexception)(&z2zE3469);
        zInvalidBlock(&z2zE3469, zRlpDecode);
        COPY(zexception)(current_exception, z2zE3469);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/lib/rlp/tx.sail:161.12-161.41");
        KILL(zexception)(&z2zE3469);
        goto end_block_exception_3442;
        /* unreachable after throw */
        KILL(zexception)(&z2zE3469);
      }
      goto finish_match_3439;
    }
  case_3440: ;
    sail_match_failure("decode_blob_hash_items");
  finish_match_3439: ;
    z8zE201 = z3zE3452;
  }
end_function_3441: ;
  return z8zE201;
end_block_exception_3442: ;

  return UINT64_C(0xdeadc0de);
}

struct zBlobHashes zdecode_blob_hashes(struct zRlpFieldRefFields zf)
{
  struct zBlobHashes z8zE202;
  struct zByteSliceFields zbytes;
  {
    struct zByteSliceFields z2zE3452;
    z2zE3452 = zf.zsource;
    uint64_t z2zE3453;
    z2zE3453 = zf.zcontent_off;
    uint64_t z2zE3454;
    z2zE3454 = zf.zcontent_len;
    zbytes = zsub_slice(z2zE3452, z2zE3453, z2zE3454);
  }
  uint64_t z2zE3451;
  {
    struct zRlpCursorFields z2zE3450;
    {
      z2zE3450 = zrlp_ref_cursor(zf);
      if (have_exception) {  goto end_block_exception_3438;  }
    }
    {
      z2zE3451 = zdecode_blob_hash_items(z2zE3450, UINT64_C(0));
      if (have_exception) {  goto end_block_exception_3438;  }
    }
  }
  struct zBlobHashes z3zE3451;
  z3zE3451.zbytes = zbytes;
  z3zE3451.zcount = z2zE3451;
  z8zE202 = z3zE3451;
end_function_3437: ;
  return z8zE202;
end_block_exception_3438: ;
  struct zByteSliceFields z8zE860 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zBlobHashes z8zE859 = { .zbytes = z8zE860, .zcount = UINT64_C(0xdeadc0de) };
  return z8zE859;
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
let_end_312: ;
}
void kill_letbind_95(void) {    KILL(zAuthorizzationDecode)(&zEMPTY_AUTHORIZATION_DECODE);
}

void zdecode_auth_tuples(struct zAuthorizzationDecode *z8zE203, struct zRlpCursorFields zcursor)
{
  bool z2zE3428;
  z2zE3428 = zrlp_cursor_empty(zcursor);
  if (z2zE3428) {
    struct zAuthorizzationDecode z3zE3449;
    CREATE(zAuthorizzationDecode)(&z3zE3449);
    zz5listz8z5structz0zzAuthorizzzzationz9 z3zE3450;
    CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&z3zE3450);
    COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&((&z3zE3449)->zauthorizzations), z3zE3450);
    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z3zE3450);
    z3zE3449.zcount = UINT64_C(0);
    COPY(zAuthorizzationDecode)((*(&z8zE203)), z3zE3449);
    KILL(zAuthorizzationDecode)(&z3zE3449);
  } else {
    struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3429;
    {
      z2zE3429 = zrlp_cursor_pop(zcursor);
      if (have_exception) {  goto end_block_exception_3436;  }
    }
    struct zAuthorizzationDecode z3zE3423;
    CREATE(zAuthorizzationDecode)(&z3zE3423);
    {
      struct zRlpFieldRefFields ztuple;
      ztuple = z2zE3429.ztup0;
      struct zRlpCursorFields znext;
      znext = z2zE3429.ztup1;
      struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3431;
      {
        struct zRlpCursorFields z2zE3430;
        {
          z2zE3430 = zrlp_ref_cursor(ztuple);
          if (have_exception) {
            KILL(zAuthorizzationDecode)(&z3zE3423);
            goto end_block_exception_3436;
          }
        }
        {
          z2zE3431 = zrlp_cursor_pop(z2zE3430);
          if (have_exception) {
            KILL(zAuthorizzationDecode)(&z3zE3423);
            goto end_block_exception_3436;
          }
        }
      }
      struct zAuthorizzationDecode z3zE3424;
      CREATE(zAuthorizzationDecode)(&z3zE3424);
      {
        struct zRlpFieldRefFields zchain_f;
        zchain_f = z2zE3431.ztup0;
        struct zRlpCursorFields zfields;
        zfields = z2zE3431.ztup1;
        struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3432;
        {
          z2zE3432 = zrlp_cursor_pop(zfields);
          if (have_exception) {
            KILL(zAuthorizzationDecode)(&z3zE3423);
            KILL(zAuthorizzationDecode)(&z3zE3424);
            goto end_block_exception_3436;
          }
        }
        struct zAuthorizzationDecode z3zE3425;
        CREATE(zAuthorizzationDecode)(&z3zE3425);
        {
          struct zRlpFieldRefFields zaddr_f;
          zaddr_f = z2zE3432.ztup0;
          struct zRlpCursorFields z1zE68;
          z1zE68 = z2zE3432.ztup1;
          struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3433;
          {
            z2zE3433 = zrlp_cursor_pop(z1zE68);
            if (have_exception) {
              KILL(zAuthorizzationDecode)(&z3zE3423);
              KILL(zAuthorizzationDecode)(&z3zE3424);
              KILL(zAuthorizzationDecode)(&z3zE3425);
              goto end_block_exception_3436;
            }
          }
          struct zAuthorizzationDecode z3zE3426;
          CREATE(zAuthorizzationDecode)(&z3zE3426);
          {
            struct zRlpFieldRefFields znonce_f;
            znonce_f = z2zE3433.ztup0;
            struct zRlpCursorFields z1zE69;
            z1zE69 = z2zE3433.ztup1;
            struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3434;
            {
              z2zE3434 = zrlp_cursor_pop(z1zE69);
              if (have_exception) {
                KILL(zAuthorizzationDecode)(&z3zE3423);
                KILL(zAuthorizzationDecode)(&z3zE3424);
                KILL(zAuthorizzationDecode)(&z3zE3425);
                KILL(zAuthorizzationDecode)(&z3zE3426);
                goto end_block_exception_3436;
              }
            }
            struct zAuthorizzationDecode z3zE3427;
            CREATE(zAuthorizzationDecode)(&z3zE3427);
            {
              struct zRlpFieldRefFields zy_f;
              zy_f = z2zE3434.ztup0;
              struct zRlpCursorFields z1zE70;
              z1zE70 = z2zE3434.ztup1;
              struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3435;
              {
                z2zE3435 = zrlp_cursor_pop(z1zE70);
                if (have_exception) {
                  KILL(zAuthorizzationDecode)(&z3zE3423);
                  KILL(zAuthorizzationDecode)(&z3zE3424);
                  KILL(zAuthorizzationDecode)(&z3zE3425);
                  KILL(zAuthorizzationDecode)(&z3zE3426);
                  KILL(zAuthorizzationDecode)(&z3zE3427);
                  goto end_block_exception_3436;
                }
              }
              struct zAuthorizzationDecode z3zE3428;
              CREATE(zAuthorizzationDecode)(&z3zE3428);
              {
                struct zRlpFieldRefFields zr_f;
                zr_f = z2zE3435.ztup0;
                struct zRlpCursorFields z1zE71;
                z1zE71 = z2zE3435.ztup1;
                struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3436;
                {
                  z2zE3436 = zrlp_cursor_pop(z1zE71);
                  if (have_exception) {
                    KILL(zAuthorizzationDecode)(&z3zE3423);
                    KILL(zAuthorizzationDecode)(&z3zE3424);
                    KILL(zAuthorizzationDecode)(&z3zE3425);
                    KILL(zAuthorizzationDecode)(&z3zE3426);
                    KILL(zAuthorizzationDecode)(&z3zE3427);
                    KILL(zAuthorizzationDecode)(&z3zE3428);
                    goto end_block_exception_3436;
                  }
                }
                struct zAuthorizzationDecode z3zE3429;
                CREATE(zAuthorizzationDecode)(&z3zE3429);
                {
                  struct zRlpFieldRefFields zs_f;
                  zs_f = z2zE3436.ztup0;
                  struct zRlpCursorFields z1zE72;
                  z1zE72 = z2zE3436.ztup1;
                  unit z3zE3430;
                  {
                    z3zE3430 = zrlp_cursor_expect_end(z1zE72);
                    if (have_exception) {
                      KILL(zAuthorizzationDecode)(&z3zE3423);
                      KILL(zAuthorizzationDecode)(&z3zE3424);
                      KILL(zAuthorizzationDecode)(&z3zE3425);
                      KILL(zAuthorizzationDecode)(&z3zE3426);
                      KILL(zAuthorizzationDecode)(&z3zE3427);
                      KILL(zAuthorizzationDecode)(&z3zE3428);
                      KILL(zAuthorizzationDecode)(&z3zE3429);
                      goto end_block_exception_3436;
                    }
                  }
                  sail_u256 zchain_id;
                  {
                    zchain_id = zrlp_ref_uint_word(zchain_f);
                    if (have_exception) {
                      KILL(zAuthorizzationDecode)(&z3zE3423);
                      KILL(zAuthorizzationDecode)(&z3zE3424);
                      KILL(zAuthorizzationDecode)(&z3zE3425);
                      KILL(zAuthorizzationDecode)(&z3zE3426);
                      KILL(zAuthorizzationDecode)(&z3zE3427);
                      KILL(zAuthorizzationDecode)(&z3zE3428);
                      KILL(zAuthorizzationDecode)(&z3zE3429);
                      goto end_block_exception_3436;
                    }
                  }
                  uint64_t zauth_nonce;
                  {
                    zauth_nonce = zrlp_ref_account_nonce(znonce_f);
                    if (have_exception) {
                      KILL(zAuthorizzationDecode)(&z3zE3423);
                      KILL(zAuthorizzationDecode)(&z3zE3424);
                      KILL(zAuthorizzationDecode)(&z3zE3425);
                      KILL(zAuthorizzationDecode)(&z3zE3426);
                      KILL(zAuthorizzationDecode)(&z3zE3427);
                      KILL(zAuthorizzationDecode)(&z3zE3428);
                      KILL(zAuthorizzationDecode)(&z3zE3429);
                      goto end_block_exception_3436;
                    }
                  }
                  uint64_t zy_value;
                  {
                    zy_value = zrlp_ref_uint64(zy_f);
                    if (have_exception) {
                      KILL(zAuthorizzationDecode)(&z3zE3423);
                      KILL(zAuthorizzationDecode)(&z3zE3424);
                      KILL(zAuthorizzationDecode)(&z3zE3425);
                      KILL(zAuthorizzationDecode)(&z3zE3426);
                      KILL(zAuthorizzationDecode)(&z3zE3427);
                      KILL(zAuthorizzationDecode)(&z3zE3428);
                      KILL(zAuthorizzationDecode)(&z3zE3429);
                      goto end_block_exception_3436;
                    }
                  }
                  bool zy_valid;
                  zy_valid = (!(UINT64_C(1) < zy_value));
                  uint64_t zy;
                  {
                    bool z2zE3449;
                    z2zE3449 = (zy_value == UINT64_C(0));
                    if (z2zE3449) {  zy = UINT64_C(0);  } else {  zy = UINT64_C(1);  }
                  }
                  sail_u256 zr;
                  {
                    zr = zrlp_ref_uint_word(zr_f);
                    if (have_exception) {
                      KILL(zAuthorizzationDecode)(&z3zE3423);
                      KILL(zAuthorizzationDecode)(&z3zE3424);
                      KILL(zAuthorizzationDecode)(&z3zE3425);
                      KILL(zAuthorizzationDecode)(&z3zE3426);
                      KILL(zAuthorizzationDecode)(&z3zE3427);
                      KILL(zAuthorizzationDecode)(&z3zE3428);
                      KILL(zAuthorizzationDecode)(&z3zE3429);
                      goto end_block_exception_3436;
                    }
                  }
                  sail_u256 zs;
                  {
                    zs = zrlp_ref_uint_word(zs_f);
                    if (have_exception) {
                      KILL(zAuthorizzationDecode)(&z3zE3423);
                      KILL(zAuthorizzationDecode)(&z3zE3424);
                      KILL(zAuthorizzationDecode)(&z3zE3425);
                      KILL(zAuthorizzationDecode)(&z3zE3426);
                      KILL(zAuthorizzationDecode)(&z3zE3427);
                      KILL(zAuthorizzationDecode)(&z3zE3428);
                      KILL(zAuthorizzationDecode)(&z3zE3429);
                      goto end_block_exception_3436;
                    }
                  }
                  sail_fixed_bytes_20 zauth_addr;
                  {
                    sail_u256 z2zE3448;
                    {
                      z2zE3448 = zrlp_ref_word(zaddr_f);
                      if (have_exception) {
                        KILL(zAuthorizzationDecode)(&z3zE3423);
                        KILL(zAuthorizzationDecode)(&z3zE3424);
                        KILL(zAuthorizzationDecode)(&z3zE3425);
                        KILL(zAuthorizzationDecode)(&z3zE3426);
                        KILL(zAuthorizzationDecode)(&z3zE3427);
                        KILL(zAuthorizzationDecode)(&z3zE3428);
                        KILL(zAuthorizzationDecode)(&z3zE3429);
                        goto end_block_exception_3436;
                      }
                    }
                    zauth_addr = zword_to_address(z2zE3448);
                  }
                  struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE3438;
                  if (zy_valid) {
                    sail_fixed_bytes_32 z2zE3437;
                    z2zE3437 = zauth_signing_hash(zchain_id, zauth_addr, zauth_nonce);
                    z2zE3438 = zecrecover_addr(z2zE3437, zy, zr, zs);
                  } else {
                    struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z3zE3431;
                    z3zE3431.ztup0 = false;
                    z3zE3431.ztup1 = zZERO_ADDRESS;
                    z2zE3438 = z3zE3431;
                  }
                  struct zAuthorizzationDecode z3zE3432;
                  CREATE(zAuthorizzationDecode)(&z3zE3432);
                  {
                    bool zok;
                    zok = z2zE3438.ztup0;
                    sail_fixed_bytes_20 zauthority;
                    zauthority = z2zE3438.ztup1;
                    bool znonce_valid;
                    znonce_valid = (zauth_nonce != UINT64_C(18446744073709551615));
                    struct zAuthorizzation zauthorizzation;
                    {
                      bool z2zE3447;
                      {
                        bool z3zE3438;
                        if (zok) {
                          bool z3zE3437;
                          if (zy_valid) {
                            bool z2zE3446;
                            z2zE3446 = zword_ult(zZERO_WORD, zr);
                            bool z3zE3436;
                            if (z2zE3446) {
                              bool z2zE3445;
                              z2zE3445 = zword_ult(zr, zSECP_N_FULL);
                              bool z3zE3435;
                              if (z2zE3445) {
                                bool z2zE3444;
                                z2zE3444 = zword_ult(zZERO_WORD, zs);
                                bool z3zE3434;
                                if (z2zE3444) {
                                  bool z2zE3443;
                                  z2zE3443 = zword_ule(zs, zSECP_N_HALF);
                                  bool z3zE3433;
                                  if (z2zE3443) {  z3zE3433 = znonce_valid;  } else {  z3zE3433 = false;  }
                                  z3zE3434 = z3zE3433;
                                } else {  z3zE3434 = false;  }
                                z3zE3435 = z3zE3434;
                              } else {  z3zE3435 = false;  }
                              z3zE3436 = z3zE3435;
                            } else {  z3zE3436 = false;  }
                            z3zE3437 = z3zE3436;
                          } else {  z3zE3437 = false;  }
                          z3zE3438 = z3zE3437;
                        } else {  z3zE3438 = false;  }
                        z2zE3447 = z3zE3438;
                      }
                      struct zAuthorizzation z3zE3439;
                      z3zE3439.zaddress = zauth_addr;
                      z3zE3439.zauthority = zauthority;
                      z3zE3439.zchain_id = zchain_id;
                      z3zE3439.znonce = zauth_nonce;
                      z3zE3439.zvalid_sig = z2zE3447;
                      zauthorizzation = z3zE3439;
                    }
                    struct zAuthorizzationDecode ztail;
                    CREATE(zAuthorizzationDecode)(&ztail);
                    {
                      zdecode_auth_tuples(&ztail, znext);
                      if (have_exception) {
                        KILL(zAuthorizzationDecode)(&z3zE3423);
                        KILL(zAuthorizzationDecode)(&z3zE3424);
                        KILL(zAuthorizzationDecode)(&z3zE3425);
                        KILL(zAuthorizzationDecode)(&z3zE3426);
                        KILL(zAuthorizzationDecode)(&z3zE3427);
                        KILL(zAuthorizzationDecode)(&z3zE3428);
                        KILL(zAuthorizzationDecode)(&z3zE3429);
                        KILL(zAuthorizzationDecode)(&z3zE3432);
                        KILL(zAuthorizzationDecode)(&ztail);
                        goto end_block_exception_3436;
                      }
                    }
                    zz5listz8z5structz0zzAuthorizzzzationz9 z2zE3440;
                    CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE3440);
                    {
                      zz5listz8z5structz0zzAuthorizzzzationz9 z2zE3439;
                      CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE3439);
                      COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE3439, ztail.zauthorizzations);
                      zconsz3z5structz0zzAuthorizzzzation(&z2zE3440, zauthorizzation, z2zE3439);
                      KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE3439);
                    }
                    uint64_t z2zE3442;
                    {
                      uint64_t z2zE3441;
                      z2zE3441 = ztail.zcount;
                      z2zE3442 = zrlp_item_count_incrementzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE3441);
                    }
                    struct zAuthorizzationDecode z3zE3440;
                    CREATE(zAuthorizzationDecode)(&z3zE3440);
                    COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&((&z3zE3440)->zauthorizzations), z2zE3440);
                    z3zE3440.zcount = z2zE3442;
                    COPY(zAuthorizzationDecode)(&z3zE3432, z3zE3440);
                    KILL(zAuthorizzationDecode)(&z3zE3440);
                    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE3440);
                    KILL(zAuthorizzationDecode)(&ztail);
                    goto finish_match_3433;
                  }
                case_3434: ;
                  sail_match_failure("decode_auth_tuples");
                finish_match_3433: ;
                  COPY(zAuthorizzationDecode)(&z3zE3429, z3zE3432);
                  KILL(zAuthorizzationDecode)(&z3zE3432);
                  goto finish_match_3431;
                }
              case_3432: ;
                sail_match_failure("decode_auth_tuples");
              finish_match_3431: ;
                COPY(zAuthorizzationDecode)(&z3zE3428, z3zE3429);
                KILL(zAuthorizzationDecode)(&z3zE3429);
                goto finish_match_3429;
              }
            case_3430: ;
              sail_match_failure("decode_auth_tuples");
            finish_match_3429: ;
              COPY(zAuthorizzationDecode)(&z3zE3427, z3zE3428);
              KILL(zAuthorizzationDecode)(&z3zE3428);
              goto finish_match_3427;
            }
          case_3428: ;
            sail_match_failure("decode_auth_tuples");
          finish_match_3427: ;
            COPY(zAuthorizzationDecode)(&z3zE3426, z3zE3427);
            KILL(zAuthorizzationDecode)(&z3zE3427);
            goto finish_match_3425;
          }
        case_3426: ;
          sail_match_failure("decode_auth_tuples");
        finish_match_3425: ;
          COPY(zAuthorizzationDecode)(&z3zE3425, z3zE3426);
          KILL(zAuthorizzationDecode)(&z3zE3426);
          goto finish_match_3423;
        }
      case_3424: ;
        sail_match_failure("decode_auth_tuples");
      finish_match_3423: ;
        COPY(zAuthorizzationDecode)(&z3zE3424, z3zE3425);
        KILL(zAuthorizzationDecode)(&z3zE3425);
        goto finish_match_3421;
      }
    case_3422: ;
      sail_match_failure("decode_auth_tuples");
    finish_match_3421: ;
      COPY(zAuthorizzationDecode)(&z3zE3423, z3zE3424);
      KILL(zAuthorizzationDecode)(&z3zE3424);
      goto finish_match_3419;
    }
  case_3420: ;
    sail_match_failure("decode_auth_tuples");
  finish_match_3419: ;
    COPY(zAuthorizzationDecode)((*(&z8zE203)), z3zE3423);
    KILL(zAuthorizzationDecode)(&z3zE3423);
  }
end_function_3435: ;
  goto end_function_4080;
end_block_exception_3436: ;
  goto end_function_4080;
end_function_4080: ;
}

void zdecode_auth_list(struct ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9 *z8zE204, struct zRlpFieldRefFields zf)
{
  bool z2zE3423;
  {
    uint64_t z2zE3422;
    {
      struct zByteSliceFields z2zE3421;
      z2zE3421 = zf.zsource;
      z2zE3422 = z2zE3421.zlen;
    }
    z2zE3423 = (!(UINT64_C(1073741824) < z2zE3422));
  }
  if (z2zE3423) {
    struct zAuthorizzationDecode zdecoded;
    CREATE(zAuthorizzationDecode)(&zdecoded);
    {
      struct zRlpCursorFields z2zE3426;
      {
        z2zE3426 = zrlp_ref_cursor(zf);
        if (have_exception) {
          KILL(zAuthorizzationDecode)(&zdecoded);
          goto end_block_exception_3418;
        }
      }
      {
        zdecode_auth_tuples(&zdecoded, z2zE3426);
        if (have_exception) {
          KILL(zAuthorizzationDecode)(&zdecoded);
          goto end_block_exception_3418;
        }
      }
    }
    zz5listz8z5structz0zzAuthorizzzzationz9 z2zE3424;
    CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE3424);
    COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE3424, zdecoded.zauthorizzations);
    uint64_t z2zE3425;
    z2zE3425 = zdecoded.zcount;
    struct ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9 z3zE3422;
    CREATE(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z3zE3422);
    COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&((&z3zE3422)->ztup0), z2zE3424);
    z3zE3422.ztup1 = z2zE3425;
    COPY(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)((*(&z8zE204)), z3zE3422);
    KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z3zE3422);
    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE3424);
    KILL(zAuthorizzationDecode)(&zdecoded);
  } else {
    struct zexception z2zE3427;
    CREATE(zexception)(&z2zE3427);
    zInvalidBlock(&z2zE3427, zRlpDecode);
    COPY(zexception)(current_exception, z2zE3427);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/tx.sail:272.8-272.37");
    KILL(zexception)(&z2zE3427);
    goto end_block_exception_3418;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3427);
  }
end_function_3417: ;
  goto end_function_4079;
end_block_exception_3418: ;
  goto end_function_4079;
end_function_4079: ;
}

struct zByteSliceFields ztx_input_span(struct zRlpFieldRefFields zdata)
{
  struct zByteSliceFields z8zE205;
  struct zByteSliceFields zcontent;
  zcontent = zrlp_ref_content(zdata);
  bool z2zE3419;
  {
    uint64_t z2zE3418;
    z2zE3418 = zcontent.zlen;
    z2zE3419 = (!(UINT64_C(1073741824) < z2zE3418));
  }
  if (z2zE3419) {  z8zE205 = zcontent;  } else {
    struct zexception z2zE3420;
    CREATE(zexception)(&z2zE3420);
    zInvalidBlock(&z2zE3420, zRlpDecode);
    COPY(zexception)(current_exception, z2zE3420);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/tx.sail:284.8-284.37");
    KILL(zexception)(&z2zE3420);
    goto end_block_exception_3416;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3420);
  }
end_function_3415: ;
  return z8zE205;
end_block_exception_3416: ;
  struct zByteSliceFields z8zE861 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE861;
}

struct zByteSliceFields ztx_sig_span(struct zRlpFieldRefFields zfirst, struct zRlpFieldRefFields zsignature)
{
  struct zByteSliceFields z8zE206;
  uint64_t zstart;
  zstart = zfirst.zfull_off;
  uint64_t zstop;
  zstop = zsignature.zfull_off;
  uint64_t zstart_offset;
  zstart_offset = zstart;
  uint64_t zstop_offset;
  zstop_offset = zstop;
  bool z2zE3414;
  z2zE3414 = (zstop_offset < zstart_offset);
  if (z2zE3414) {
    struct zexception z2zE3415;
    CREATE(zexception)(&z2zE3415);
    zInvalidBlock(&z2zE3415, zRlpDecode);
    COPY(zexception)(current_exception, z2zE3415);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/tx.sail:343.8-343.37");
    KILL(zexception)(&z2zE3415);
    goto end_block_exception_3414;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3415);
  } else {
    struct zByteSliceFields z2zE3416;
    z2zE3416 = zfirst.zsource;
    uint64_t z2zE3417;
    {    z2zE3417 = (zstop_offset - zstart_offset);
    }
    z8zE206 = zsub_slice(z2zE3416, zstart, z2zE3417);
  }
end_function_3413: ;
  return z8zE206;
end_block_exception_3414: ;
  struct zByteSliceFields z8zE862 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE862;
}

uint64_t zrlp_ref_gas(struct zRlpFieldRefFields zf, enum zFork zfork)
{
  uint64_t z8zE207;
  uint64_t zvalue;
  {
    zvalue = zrlp_ref_uint64(zf);
    if (have_exception) {  goto end_block_exception_3412;  }
  }
  bool z2zE3412;
  {
    bool z2zE3411;
    z2zE3411 = zfork_gteq(zfork, zOsaka);
    bool z3zE3421;
    if (z2zE3411) {
      bool z2zE3410;
      z2zE3410 = zfork_lt(zfork, zAmsterdam);
      bool z3zE3420;
      if (z2zE3410) {  z3zE3420 = (zOSAKA_TRANSACTION_GAS_LIMIT_VALUE < zvalue);  } else {  z3zE3420 = false;  }
      z3zE3421 = z3zE3420;
    } else {  z3zE3421 = false;  }
    z2zE3412 = z3zE3421;
  }
  if (z2zE3412) {
    struct zexception z2zE3413;
    CREATE(zexception)(&z2zE3413);
    zInvalidBlock(&z2zE3413, zGasUsedExceedsLimit);
    COPY(zexception)(current_exception, z2zE3413);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/splices/c_optimized.sail:198.8-198.47");
    KILL(zexception)(&z2zE3413);
    goto end_block_exception_3412;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3413);
  } else {  z8zE207 = zvalue;  }
end_function_3411: ;
  return z8zE207;
end_block_exception_3412: ;

  return UINT64_C(0xdeadc0de);
}

void zrlp_decode_tx(struct zTransaction *z8zE208, struct zByteSliceFields ztx, struct zByteSliceFields zpubkey, enum zFork zfork)
{
  sail_fixed_bytes_20 zsender;
  {
    sail_u256 z2zE3409;
    {
      sail_fixed_bytes_32 z2zE3408;
      {
        struct zByteSliceFields z2zE3407;
        z2zE3407 = zsub_slice(zpubkey, UINT64_C(1), zPUBLIC_KEY_BODY_LENGTH);
        z2zE3408 = zkeccak256_slice(z2zE3407);
      }
      z2zE3409 = zhash_to_word(z2zE3408);
    }
    zsender = zword_to_address(z2zE3409);
  }
  uint64_t ztx_length;
  ztx_length = ztx.zlen;
  uint64_t zb0;
  {
    bool z2zE3405;
    z2zE3405 = (ztx_length == UINT64_C(0));
    if (z2zE3405) {
      struct zexception z2zE3406;
      CREATE(zexception)(&z2zE3406);
      zInvalidBlock(&z2zE3406, zRlpDecode);
      COPY(zexception)(current_exception, z2zE3406);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/lib/rlp/tx.sail:391.12-391.41");
      KILL(zexception)(&z2zE3406);
      goto end_block_exception_3410;
      /* unreachable after throw */
      KILL(zexception)(&z2zE3406);
    } else {  zb0 = zslice_byte(ztx, UINT64_C(0));  }
  }
  uint64_t zttype;
  {
    bool z2zE3404;
    {
      uint64_t z2zE3403;
      z2zE3403 = (safe_rshift(UINT64_MAX, 64 - 2) & (zb0 >> UINT64_C(6)));
      z2zE3404 = (z2zE3403 == UINT64_C(0b11));
    }
    if (z2zE3404) {  zttype = UINT64_C(0x00);  } else {  zttype = zb0;  }
  }
  bool ztyped;
  ztyped = (zttype != UINT64_C(0x00));
  struct zByteSliceFields zpayload;
  if (ztyped) {
    bool z2zE3400;
    z2zE3400 = (!(ztx_length < UINT64_C(1)));
    if (z2zE3400) {
      uint64_t z2zE3401;
      {    z2zE3401 = (ztx_length - UINT64_C(1));
      }
      zpayload = zsub_slice(ztx, UINT64_C(1), z2zE3401);
    } else {
      struct zexception z2zE3402;
      CREATE(zexception)(&z2zE3402);
      zInvalidBlock(&z2zE3402, zRlpDecode);
      COPY(zexception)(current_exception, z2zE3402);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/lib/rlp/tx.sail:401.16-401.45");
      KILL(zexception)(&z2zE3402);
      goto end_block_exception_3410;
      /* unreachable after throw */
      KILL(zexception)(&z2zE3402);
    }
  } else {  zpayload = ztx;  }
  struct zRlpCursorFields zfields;
  {
    zfields = zrlp_node_cursor(zpayload);
    if (have_exception) {  goto end_block_exception_3410;  }
  }
  struct zTransaction z3zE3205;
  CREATE(zTransaction)(&z3zE3205);
  {
    uint64_t zp0z3;
    zp0z3 = zttype;
    bool z3zE3355;
    z3zE3355 = (zp0z3 == UINT64_C(0x00));
    if (!(z3zE3355)) {  goto case_3390;  }
    struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3263;
    {
      z2zE3263 = zrlp_cursor_pop(zfields);
      if (have_exception) {
        KILL(zTransaction)(&z3zE3205);
        goto end_block_exception_3410;
      }
    }
    struct zTransaction z3zE3332;
    CREATE(zTransaction)(&z3zE3332);
    {
      struct zRlpFieldRefFields znonce_f;
      znonce_f = z2zE3263.ztup0;
      struct zRlpCursorFields z1zE9;
      z1zE9 = z2zE3263.ztup1;
      struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3264;
      {
        z2zE3264 = zrlp_cursor_pop(z1zE9);
        if (have_exception) {
          KILL(zTransaction)(&z3zE3205);
          KILL(zTransaction)(&z3zE3332);
          goto end_block_exception_3410;
        }
      }
      struct zTransaction z3zE3333;
      CREATE(zTransaction)(&z3zE3333);
      {
        struct zRlpFieldRefFields zgp_f;
        zgp_f = z2zE3264.ztup0;
        struct zRlpCursorFields z1zE10;
        z1zE10 = z2zE3264.ztup1;
        struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3265;
        {
          z2zE3265 = zrlp_cursor_pop(z1zE10);
          if (have_exception) {
            KILL(zTransaction)(&z3zE3205);
            KILL(zTransaction)(&z3zE3332);
            KILL(zTransaction)(&z3zE3333);
            goto end_block_exception_3410;
          }
        }
        struct zTransaction z3zE3334;
        CREATE(zTransaction)(&z3zE3334);
        {
          struct zRlpFieldRefFields zgas_f;
          zgas_f = z2zE3265.ztup0;
          struct zRlpCursorFields z1zE11;
          z1zE11 = z2zE3265.ztup1;
          struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3266;
          {
            z2zE3266 = zrlp_cursor_pop(z1zE11);
            if (have_exception) {
              KILL(zTransaction)(&z3zE3205);
              KILL(zTransaction)(&z3zE3332);
              KILL(zTransaction)(&z3zE3333);
              KILL(zTransaction)(&z3zE3334);
              goto end_block_exception_3410;
            }
          }
          struct zTransaction z3zE3335;
          CREATE(zTransaction)(&z3zE3335);
          {
            struct zRlpFieldRefFields zto_f;
            zto_f = z2zE3266.ztup0;
            struct zRlpCursorFields z1zE12;
            z1zE12 = z2zE3266.ztup1;
            struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3267;
            {
              z2zE3267 = zrlp_cursor_pop(z1zE12);
              if (have_exception) {
                KILL(zTransaction)(&z3zE3205);
                KILL(zTransaction)(&z3zE3332);
                KILL(zTransaction)(&z3zE3333);
                KILL(zTransaction)(&z3zE3334);
                KILL(zTransaction)(&z3zE3335);
                goto end_block_exception_3410;
              }
            }
            struct zTransaction z3zE3336;
            CREATE(zTransaction)(&z3zE3336);
            {
              struct zRlpFieldRefFields zvalue_f;
              zvalue_f = z2zE3267.ztup0;
              struct zRlpCursorFields z1zE13;
              z1zE13 = z2zE3267.ztup1;
              struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3268;
              {
                z2zE3268 = zrlp_cursor_pop(z1zE13);
                if (have_exception) {
                  KILL(zTransaction)(&z3zE3205);
                  KILL(zTransaction)(&z3zE3332);
                  KILL(zTransaction)(&z3zE3333);
                  KILL(zTransaction)(&z3zE3334);
                  KILL(zTransaction)(&z3zE3335);
                  KILL(zTransaction)(&z3zE3336);
                  goto end_block_exception_3410;
                }
              }
              struct zTransaction z3zE3337;
              CREATE(zTransaction)(&z3zE3337);
              {
                struct zRlpFieldRefFields zdata_f;
                zdata_f = z2zE3268.ztup0;
                struct zRlpCursorFields z1zE14;
                z1zE14 = z2zE3268.ztup1;
                struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3269;
                {
                  z2zE3269 = zrlp_cursor_pop(z1zE14);
                  if (have_exception) {
                    KILL(zTransaction)(&z3zE3205);
                    KILL(zTransaction)(&z3zE3332);
                    KILL(zTransaction)(&z3zE3333);
                    KILL(zTransaction)(&z3zE3334);
                    KILL(zTransaction)(&z3zE3335);
                    KILL(zTransaction)(&z3zE3336);
                    KILL(zTransaction)(&z3zE3337);
                    goto end_block_exception_3410;
                  }
                }
                struct zTransaction z3zE3338;
                CREATE(zTransaction)(&z3zE3338);
                {
                  struct zRlpFieldRefFields zv_f;
                  zv_f = z2zE3269.ztup0;
                  struct zRlpCursorFields z1zE15;
                  z1zE15 = z2zE3269.ztup1;
                  struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3270;
                  {
                    z2zE3270 = zrlp_cursor_pop(z1zE15);
                    if (have_exception) {
                      KILL(zTransaction)(&z3zE3205);
                      KILL(zTransaction)(&z3zE3332);
                      KILL(zTransaction)(&z3zE3333);
                      KILL(zTransaction)(&z3zE3334);
                      KILL(zTransaction)(&z3zE3335);
                      KILL(zTransaction)(&z3zE3336);
                      KILL(zTransaction)(&z3zE3337);
                      KILL(zTransaction)(&z3zE3338);
                      goto end_block_exception_3410;
                    }
                  }
                  struct zTransaction z3zE3339;
                  CREATE(zTransaction)(&z3zE3339);
                  {
                    struct zRlpFieldRefFields zr_f;
                    zr_f = z2zE3270.ztup0;
                    struct zRlpCursorFields z1zE16;
                    z1zE16 = z2zE3270.ztup1;
                    struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3271;
                    {
                      z2zE3271 = zrlp_cursor_pop(z1zE16);
                      if (have_exception) {
                        KILL(zTransaction)(&z3zE3205);
                        KILL(zTransaction)(&z3zE3332);
                        KILL(zTransaction)(&z3zE3333);
                        KILL(zTransaction)(&z3zE3334);
                        KILL(zTransaction)(&z3zE3335);
                        KILL(zTransaction)(&z3zE3336);
                        KILL(zTransaction)(&z3zE3337);
                        KILL(zTransaction)(&z3zE3338);
                        KILL(zTransaction)(&z3zE3339);
                        goto end_block_exception_3410;
                      }
                    }
                    struct zTransaction z3zE3340;
                    CREATE(zTransaction)(&z3zE3340);
                    {
                      struct zRlpFieldRefFields zs_f;
                      zs_f = z2zE3271.ztup0;
                      struct zRlpCursorFields z1zE17;
                      z1zE17 = z2zE3271.ztup1;
                      unit z3zE3341;
                      {
                        z3zE3341 = zrlp_cursor_expect_end(z1zE17);
                        if (have_exception) {
                          KILL(zTransaction)(&z3zE3205);
                          KILL(zTransaction)(&z3zE3332);
                          KILL(zTransaction)(&z3zE3333);
                          KILL(zTransaction)(&z3zE3334);
                          KILL(zTransaction)(&z3zE3335);
                          KILL(zTransaction)(&z3zE3336);
                          KILL(zTransaction)(&z3zE3337);
                          KILL(zTransaction)(&z3zE3338);
                          KILL(zTransaction)(&z3zE3339);
                          KILL(zTransaction)(&z3zE3340);
                          goto end_block_exception_3410;
                        }
                      }
                      sail_u256 zv;
                      {
                        zv = zrlp_ref_word(zv_f);
                        if (have_exception) {
                          KILL(zTransaction)(&z3zE3205);
                          KILL(zTransaction)(&z3zE3332);
                          KILL(zTransaction)(&z3zE3333);
                          KILL(zTransaction)(&z3zE3334);
                          KILL(zTransaction)(&z3zE3335);
                          KILL(zTransaction)(&z3zE3336);
                          KILL(zTransaction)(&z3zE3337);
                          KILL(zTransaction)(&z3zE3338);
                          KILL(zTransaction)(&z3zE3339);
                          KILL(zTransaction)(&z3zE3340);
                          goto end_block_exception_3410;
                        }
                      }
                      sail_u256 zgp;
                      {
                        zgp = zrlp_ref_uint_word(zgp_f);
                        if (have_exception) {
                          KILL(zTransaction)(&z3zE3205);
                          KILL(zTransaction)(&z3zE3332);
                          KILL(zTransaction)(&z3zE3333);
                          KILL(zTransaction)(&z3zE3334);
                          KILL(zTransaction)(&z3zE3335);
                          KILL(zTransaction)(&z3zE3336);
                          KILL(zTransaction)(&z3zE3337);
                          KILL(zTransaction)(&z3zE3338);
                          KILL(zTransaction)(&z3zE3339);
                          KILL(zTransaction)(&z3zE3340);
                          goto end_block_exception_3410;
                        }
                      }
                      sail_u256 z2zE3272;
                      {
                        z2zE3272 = zrlp_ref_uint_word(znonce_f);
                        if (have_exception) {
                          KILL(zTransaction)(&z3zE3205);
                          KILL(zTransaction)(&z3zE3332);
                          KILL(zTransaction)(&z3zE3333);
                          KILL(zTransaction)(&z3zE3334);
                          KILL(zTransaction)(&z3zE3335);
                          KILL(zTransaction)(&z3zE3336);
                          KILL(zTransaction)(&z3zE3337);
                          KILL(zTransaction)(&z3zE3338);
                          KILL(zTransaction)(&z3zE3339);
                          KILL(zTransaction)(&z3zE3340);
                          goto end_block_exception_3410;
                        }
                      }
                      uint64_t z2zE3273;
                      {
                        z2zE3273 = zrlp_ref_gas(zgas_f, zfork);
                        if (have_exception) {
                          KILL(zTransaction)(&z3zE3205);
                          KILL(zTransaction)(&z3zE3332);
                          KILL(zTransaction)(&z3zE3333);
                          KILL(zTransaction)(&z3zE3334);
                          KILL(zTransaction)(&z3zE3335);
                          KILL(zTransaction)(&z3zE3336);
                          KILL(zTransaction)(&z3zE3337);
                          KILL(zTransaction)(&z3zE3338);
                          KILL(zTransaction)(&z3zE3339);
                          KILL(zTransaction)(&z3zE3340);
                          goto end_block_exception_3410;
                        }
                      }
                      bool z2zE3275;
                      {
                        uint64_t z2zE3274;
                        z2zE3274 = zto_f.zcontent_len;
                        z2zE3275 = (z2zE3274 == UINT64_C(0));
                      }
                      sail_fixed_bytes_20 z2zE3277;
                      {
                        sail_u256 z2zE3276;
                        {
                          z2zE3276 = zrlp_ref_word(zto_f);
                          if (have_exception) {
                            KILL(zTransaction)(&z3zE3205);
                            KILL(zTransaction)(&z3zE3332);
                            KILL(zTransaction)(&z3zE3333);
                            KILL(zTransaction)(&z3zE3334);
                            KILL(zTransaction)(&z3zE3335);
                            KILL(zTransaction)(&z3zE3336);
                            KILL(zTransaction)(&z3zE3337);
                            KILL(zTransaction)(&z3zE3338);
                            KILL(zTransaction)(&z3zE3339);
                            KILL(zTransaction)(&z3zE3340);
                            goto end_block_exception_3410;
                          }
                        }
                        z2zE3277 = zword_to_address(z2zE3276);
                      }
                      sail_u256 z2zE3278;
                      {
                        z2zE3278 = zrlp_ref_uint_word(zvalue_f);
                        if (have_exception) {
                          KILL(zTransaction)(&z3zE3205);
                          KILL(zTransaction)(&z3zE3332);
                          KILL(zTransaction)(&z3zE3333);
                          KILL(zTransaction)(&z3zE3334);
                          KILL(zTransaction)(&z3zE3335);
                          KILL(zTransaction)(&z3zE3336);
                          KILL(zTransaction)(&z3zE3337);
                          KILL(zTransaction)(&z3zE3338);
                          KILL(zTransaction)(&z3zE3339);
                          KILL(zTransaction)(&z3zE3340);
                          goto end_block_exception_3410;
                        }
                      }
                      struct zByteSliceFields z2zE3279;
                      {
                        z2zE3279 = ztx_input_span(zdata_f);
                        if (have_exception) {
                          KILL(zTransaction)(&z3zE3205);
                          KILL(zTransaction)(&z3zE3332);
                          KILL(zTransaction)(&z3zE3333);
                          KILL(zTransaction)(&z3zE3334);
                          KILL(zTransaction)(&z3zE3335);
                          KILL(zTransaction)(&z3zE3336);
                          KILL(zTransaction)(&z3zE3337);
                          KILL(zTransaction)(&z3zE3338);
                          KILL(zTransaction)(&z3zE3339);
                          KILL(zTransaction)(&z3zE3340);
                          goto end_block_exception_3410;
                        }
                      }
                      sail_fixed_bytes_32 z2zE3281;
                      {
                        struct zByteSliceFields z2zE3280;
                        {
                          z2zE3280 = ztx_sig_span(znonce_f, zv_f);
                          if (have_exception) {
                            KILL(zTransaction)(&z3zE3205);
                            KILL(zTransaction)(&z3zE3332);
                            KILL(zTransaction)(&z3zE3333);
                            KILL(zTransaction)(&z3zE3334);
                            KILL(zTransaction)(&z3zE3335);
                            KILL(zTransaction)(&z3zE3336);
                            KILL(zTransaction)(&z3zE3337);
                            KILL(zTransaction)(&z3zE3338);
                            KILL(zTransaction)(&z3zE3339);
                            KILL(zTransaction)(&z3zE3340);
                            goto end_block_exception_3410;
                          }
                        }
                        {
                          z2zE3281 = ztx_signing_hash(zLegacyTx, z2zE3280, zv);
                          if (have_exception) {
                            KILL(zTransaction)(&z3zE3205);
                            KILL(zTransaction)(&z3zE3332);
                            KILL(zTransaction)(&z3zE3333);
                            KILL(zTransaction)(&z3zE3334);
                            KILL(zTransaction)(&z3zE3335);
                            KILL(zTransaction)(&z3zE3336);
                            KILL(zTransaction)(&z3zE3337);
                            KILL(zTransaction)(&z3zE3338);
                            KILL(zTransaction)(&z3zE3339);
                            KILL(zTransaction)(&z3zE3340);
                            goto end_block_exception_3410;
                          }
                        }
                      }
                      sail_u256 z2zE3282;
                      {
                        z2zE3282 = zrlp_ref_uint_word(zr_f);
                        if (have_exception) {
                          KILL(zTransaction)(&z3zE3205);
                          KILL(zTransaction)(&z3zE3332);
                          KILL(zTransaction)(&z3zE3333);
                          KILL(zTransaction)(&z3zE3334);
                          KILL(zTransaction)(&z3zE3335);
                          KILL(zTransaction)(&z3zE3336);
                          KILL(zTransaction)(&z3zE3337);
                          KILL(zTransaction)(&z3zE3338);
                          KILL(zTransaction)(&z3zE3339);
                          KILL(zTransaction)(&z3zE3340);
                          goto end_block_exception_3410;
                        }
                      }
                      sail_u256 z2zE3283;
                      {
                        z2zE3283 = zrlp_ref_uint_word(zs_f);
                        if (have_exception) {
                          KILL(zTransaction)(&z3zE3205);
                          KILL(zTransaction)(&z3zE3332);
                          KILL(zTransaction)(&z3zE3333);
                          KILL(zTransaction)(&z3zE3334);
                          KILL(zTransaction)(&z3zE3335);
                          KILL(zTransaction)(&z3zE3336);
                          KILL(zTransaction)(&z3zE3337);
                          KILL(zTransaction)(&z3zE3338);
                          KILL(zTransaction)(&z3zE3339);
                          KILL(zTransaction)(&z3zE3340);
                          goto end_block_exception_3410;
                        }
                      }
                      z3zE3340.zaccess_list_address_count = UINT64_C(0);
                      zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z3zE3343;
                      CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3343);
                      COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE3340)->zaccess_list_addresses), z3zE3343);
                      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3343);
                      z3zE3340.zaccess_list_slot_count = UINT64_C(0);
                      zz5listz8z5structz0zzStorageKeyz9 z3zE3344;
                      CREATE(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3344);
                      COPY(zz5listz8z5structz0zzStorageKeyz9)(&((&z3zE3340)->zaccess_list_slots), z3zE3344);
                      KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3344);
                      z3zE3340.zauthorizzation_count = UINT64_C(0);
                      zz5listz8z5structz0zzAuthorizzzzationz9 z3zE3345;
                      CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&z3zE3345);
                      COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&((&z3zE3340)->zauthorizzations), z3zE3345);
                      KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z3zE3345);
                      z3zE3340.zblob_hashes = zEMPTY_BLOB_HASHES;
                      z3zE3340.zchain_id = UINT64_C(0);
                      z3zE3340.zgas_limit = z2zE3273;
                      z3zE3340.zinput_src = z2zE3279;
                      z3zE3340.zis_create = z2zE3275;
                      z3zE3340.zmax_blob_fee = zZERO_WORD;
                      z3zE3340.zmax_fee = zgp;
                      z3zE3340.zmax_priority_fee = zgp;
                      z3zE3340.znonce = z2zE3272;
                      z3zE3340.zpubkey = zpubkey;
                      z3zE3340.zraw = ztx;
                      z3zE3340.zrecipient = z2zE3277;
                      z3zE3340.zsender = zsender;
                      z3zE3340.zsig_r = z2zE3282;
                      z3zE3340.zsig_s = z2zE3283;
                      z3zE3340.zsig_v = zv;
                      z3zE3340.zsigning_hash = z2zE3281;
                      z3zE3340.ztx_type = zLegacyTx;
                      z3zE3340.zvalue = z2zE3278;
                      goto finish_match_3407;
                    }
                  case_3408: ;
                    sail_match_failure("rlp_decode_tx");
                  finish_match_3407: ;
                    COPY(zTransaction)(&z3zE3339, z3zE3340);
                    KILL(zTransaction)(&z3zE3340);
                    goto finish_match_3405;
                  }
                case_3406: ;
                  sail_match_failure("rlp_decode_tx");
                finish_match_3405: ;
                  COPY(zTransaction)(&z3zE3338, z3zE3339);
                  KILL(zTransaction)(&z3zE3339);
                  goto finish_match_3403;
                }
              case_3404: ;
                sail_match_failure("rlp_decode_tx");
              finish_match_3403: ;
                COPY(zTransaction)(&z3zE3337, z3zE3338);
                KILL(zTransaction)(&z3zE3338);
                goto finish_match_3401;
              }
            case_3402: ;
              sail_match_failure("rlp_decode_tx");
            finish_match_3401: ;
              COPY(zTransaction)(&z3zE3336, z3zE3337);
              KILL(zTransaction)(&z3zE3337);
              goto finish_match_3399;
            }
          case_3400: ;
            sail_match_failure("rlp_decode_tx");
          finish_match_3399: ;
            COPY(zTransaction)(&z3zE3335, z3zE3336);
            KILL(zTransaction)(&z3zE3336);
            goto finish_match_3397;
          }
        case_3398: ;
          sail_match_failure("rlp_decode_tx");
        finish_match_3397: ;
          COPY(zTransaction)(&z3zE3334, z3zE3335);
          KILL(zTransaction)(&z3zE3335);
          goto finish_match_3395;
        }
      case_3396: ;
        sail_match_failure("rlp_decode_tx");
      finish_match_3395: ;
        COPY(zTransaction)(&z3zE3333, z3zE3334);
        KILL(zTransaction)(&z3zE3334);
        goto finish_match_3393;
      }
    case_3394: ;
      sail_match_failure("rlp_decode_tx");
    finish_match_3393: ;
      COPY(zTransaction)(&z3zE3332, z3zE3333);
      KILL(zTransaction)(&z3zE3333);
      goto finish_match_3391;
    }
  case_3392: ;
    sail_match_failure("rlp_decode_tx");
  finish_match_3391: ;
    COPY(zTransaction)(&z3zE3205, z3zE3332);
    KILL(zTransaction)(&z3zE3332);
    goto finish_match_3274;
  }
case_3390: ;
  {
    uint64_t z3zE3356;
    z3zE3356 = zttype;
    bool z3zE3331;
    z3zE3331 = (z3zE3356 == UINT64_C(0x01));
    if (!(z3zE3331)) {  goto case_3365;  }
    struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3284;
    {
      z2zE3284 = zrlp_cursor_pop(zfields);
      if (have_exception) {
        KILL(zTransaction)(&z3zE3205);
        goto end_block_exception_3410;
      }
    }
    struct zTransaction z3zE3304;
    CREATE(zTransaction)(&z3zE3304);
    {
      struct zRlpFieldRefFields zchain_f;
      zchain_f = z2zE3284.ztup0;
      struct zRlpCursorFields z1zE18;
      z1zE18 = z2zE3284.ztup1;
      struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3285;
      {
        z2zE3285 = zrlp_cursor_pop(z1zE18);
        if (have_exception) {
          KILL(zTransaction)(&z3zE3205);
          KILL(zTransaction)(&z3zE3304);
          goto end_block_exception_3410;
        }
      }
      struct zTransaction z3zE3305;
      CREATE(zTransaction)(&z3zE3305);
      {
        struct zRlpFieldRefFields z3zE3357;
        z3zE3357 = z2zE3285.ztup0;
        struct zRlpCursorFields z1zE19;
        z1zE19 = z2zE3285.ztup1;
        struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3286;
        {
          z2zE3286 = zrlp_cursor_pop(z1zE19);
          if (have_exception) {
            KILL(zTransaction)(&z3zE3205);
            KILL(zTransaction)(&z3zE3304);
            KILL(zTransaction)(&z3zE3305);
            goto end_block_exception_3410;
          }
        }
        struct zTransaction z3zE3306;
        CREATE(zTransaction)(&z3zE3306);
        {
          struct zRlpFieldRefFields z3zE3358;
          z3zE3358 = z2zE3286.ztup0;
          struct zRlpCursorFields z1zE20;
          z1zE20 = z2zE3286.ztup1;
          struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3287;
          {
            z2zE3287 = zrlp_cursor_pop(z1zE20);
            if (have_exception) {
              KILL(zTransaction)(&z3zE3205);
              KILL(zTransaction)(&z3zE3304);
              KILL(zTransaction)(&z3zE3305);
              KILL(zTransaction)(&z3zE3306);
              goto end_block_exception_3410;
            }
          }
          struct zTransaction z3zE3307;
          CREATE(zTransaction)(&z3zE3307);
          {
            struct zRlpFieldRefFields z3zE3359;
            z3zE3359 = z2zE3287.ztup0;
            struct zRlpCursorFields z1zE21;
            z1zE21 = z2zE3287.ztup1;
            struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3288;
            {
              z2zE3288 = zrlp_cursor_pop(z1zE21);
              if (have_exception) {
                KILL(zTransaction)(&z3zE3205);
                KILL(zTransaction)(&z3zE3304);
                KILL(zTransaction)(&z3zE3305);
                KILL(zTransaction)(&z3zE3306);
                KILL(zTransaction)(&z3zE3307);
                goto end_block_exception_3410;
              }
            }
            struct zTransaction z3zE3308;
            CREATE(zTransaction)(&z3zE3308);
            {
              struct zRlpFieldRefFields z3zE3360;
              z3zE3360 = z2zE3288.ztup0;
              struct zRlpCursorFields z1zE22;
              z1zE22 = z2zE3288.ztup1;
              struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3289;
              {
                z2zE3289 = zrlp_cursor_pop(z1zE22);
                if (have_exception) {
                  KILL(zTransaction)(&z3zE3205);
                  KILL(zTransaction)(&z3zE3304);
                  KILL(zTransaction)(&z3zE3305);
                  KILL(zTransaction)(&z3zE3306);
                  KILL(zTransaction)(&z3zE3307);
                  KILL(zTransaction)(&z3zE3308);
                  goto end_block_exception_3410;
                }
              }
              struct zTransaction z3zE3309;
              CREATE(zTransaction)(&z3zE3309);
              {
                struct zRlpFieldRefFields z3zE3361;
                z3zE3361 = z2zE3289.ztup0;
                struct zRlpCursorFields z1zE23;
                z1zE23 = z2zE3289.ztup1;
                struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3290;
                {
                  z2zE3290 = zrlp_cursor_pop(z1zE23);
                  if (have_exception) {
                    KILL(zTransaction)(&z3zE3205);
                    KILL(zTransaction)(&z3zE3304);
                    KILL(zTransaction)(&z3zE3305);
                    KILL(zTransaction)(&z3zE3306);
                    KILL(zTransaction)(&z3zE3307);
                    KILL(zTransaction)(&z3zE3308);
                    KILL(zTransaction)(&z3zE3309);
                    goto end_block_exception_3410;
                  }
                }
                struct zTransaction z3zE3310;
                CREATE(zTransaction)(&z3zE3310);
                {
                  struct zRlpFieldRefFields z3zE3362;
                  z3zE3362 = z2zE3290.ztup0;
                  struct zRlpCursorFields z1zE24;
                  z1zE24 = z2zE3290.ztup1;
                  struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3291;
                  {
                    z2zE3291 = zrlp_cursor_pop(z1zE24);
                    if (have_exception) {
                      KILL(zTransaction)(&z3zE3205);
                      KILL(zTransaction)(&z3zE3304);
                      KILL(zTransaction)(&z3zE3305);
                      KILL(zTransaction)(&z3zE3306);
                      KILL(zTransaction)(&z3zE3307);
                      KILL(zTransaction)(&z3zE3308);
                      KILL(zTransaction)(&z3zE3309);
                      KILL(zTransaction)(&z3zE3310);
                      goto end_block_exception_3410;
                    }
                  }
                  struct zTransaction z3zE3311;
                  CREATE(zTransaction)(&z3zE3311);
                  {
                    struct zRlpFieldRefFields zal_f;
                    zal_f = z2zE3291.ztup0;
                    struct zRlpCursorFields z1zE25;
                    z1zE25 = z2zE3291.ztup1;
                    struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3292;
                    {
                      z2zE3292 = zrlp_cursor_pop(z1zE25);
                      if (have_exception) {
                        KILL(zTransaction)(&z3zE3205);
                        KILL(zTransaction)(&z3zE3304);
                        KILL(zTransaction)(&z3zE3305);
                        KILL(zTransaction)(&z3zE3306);
                        KILL(zTransaction)(&z3zE3307);
                        KILL(zTransaction)(&z3zE3308);
                        KILL(zTransaction)(&z3zE3309);
                        KILL(zTransaction)(&z3zE3310);
                        KILL(zTransaction)(&z3zE3311);
                        goto end_block_exception_3410;
                      }
                    }
                    struct zTransaction z3zE3312;
                    CREATE(zTransaction)(&z3zE3312);
                    {
                      struct zRlpFieldRefFields z3zE3363;
                      z3zE3363 = z2zE3292.ztup0;
                      struct zRlpCursorFields z1zE26;
                      z1zE26 = z2zE3292.ztup1;
                      struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3293;
                      {
                        z2zE3293 = zrlp_cursor_pop(z1zE26);
                        if (have_exception) {
                          KILL(zTransaction)(&z3zE3205);
                          KILL(zTransaction)(&z3zE3304);
                          KILL(zTransaction)(&z3zE3305);
                          KILL(zTransaction)(&z3zE3306);
                          KILL(zTransaction)(&z3zE3307);
                          KILL(zTransaction)(&z3zE3308);
                          KILL(zTransaction)(&z3zE3309);
                          KILL(zTransaction)(&z3zE3310);
                          KILL(zTransaction)(&z3zE3311);
                          KILL(zTransaction)(&z3zE3312);
                          goto end_block_exception_3410;
                        }
                      }
                      struct zTransaction z3zE3313;
                      CREATE(zTransaction)(&z3zE3313);
                      {
                        struct zRlpFieldRefFields z3zE3364;
                        z3zE3364 = z2zE3293.ztup0;
                        struct zRlpCursorFields z1zE27;
                        z1zE27 = z2zE3293.ztup1;
                        struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3294;
                        {
                          z2zE3294 = zrlp_cursor_pop(z1zE27);
                          if (have_exception) {
                            KILL(zTransaction)(&z3zE3205);
                            KILL(zTransaction)(&z3zE3304);
                            KILL(zTransaction)(&z3zE3305);
                            KILL(zTransaction)(&z3zE3306);
                            KILL(zTransaction)(&z3zE3307);
                            KILL(zTransaction)(&z3zE3308);
                            KILL(zTransaction)(&z3zE3309);
                            KILL(zTransaction)(&z3zE3310);
                            KILL(zTransaction)(&z3zE3311);
                            KILL(zTransaction)(&z3zE3312);
                            KILL(zTransaction)(&z3zE3313);
                            goto end_block_exception_3410;
                          }
                        }
                        struct zTransaction z3zE3314;
                        CREATE(zTransaction)(&z3zE3314);
                        {
                          struct zRlpFieldRefFields z3zE3365;
                          z3zE3365 = z2zE3294.ztup0;
                          struct zRlpCursorFields z1zE28;
                          z1zE28 = z2zE3294.ztup1;
                          unit z3zE3315;
                          {
                            z3zE3315 = zrlp_cursor_expect_end(z1zE28);
                            if (have_exception) {
                              KILL(zTransaction)(&z3zE3205);
                              KILL(zTransaction)(&z3zE3304);
                              KILL(zTransaction)(&z3zE3305);
                              KILL(zTransaction)(&z3zE3306);
                              KILL(zTransaction)(&z3zE3307);
                              KILL(zTransaction)(&z3zE3308);
                              KILL(zTransaction)(&z3zE3309);
                              KILL(zTransaction)(&z3zE3310);
                              KILL(zTransaction)(&z3zE3311);
                              KILL(zTransaction)(&z3zE3312);
                              KILL(zTransaction)(&z3zE3313);
                              KILL(zTransaction)(&z3zE3314);
                              goto end_block_exception_3410;
                            }
                          }
                          sail_u256 z3zE3366;
                          {
                            z3zE3366 = zrlp_ref_word(z3zE3363);
                            if (have_exception) {
                              KILL(zTransaction)(&z3zE3205);
                              KILL(zTransaction)(&z3zE3304);
                              KILL(zTransaction)(&z3zE3305);
                              KILL(zTransaction)(&z3zE3306);
                              KILL(zTransaction)(&z3zE3307);
                              KILL(zTransaction)(&z3zE3308);
                              KILL(zTransaction)(&z3zE3309);
                              KILL(zTransaction)(&z3zE3310);
                              KILL(zTransaction)(&z3zE3311);
                              KILL(zTransaction)(&z3zE3312);
                              KILL(zTransaction)(&z3zE3313);
                              KILL(zTransaction)(&z3zE3314);
                              goto end_block_exception_3410;
                            }
                          }
                          sail_u256 z3zE3367;
                          {
                            z3zE3367 = zrlp_ref_uint_word(z3zE3358);
                            if (have_exception) {
                              KILL(zTransaction)(&z3zE3205);
                              KILL(zTransaction)(&z3zE3304);
                              KILL(zTransaction)(&z3zE3305);
                              KILL(zTransaction)(&z3zE3306);
                              KILL(zTransaction)(&z3zE3307);
                              KILL(zTransaction)(&z3zE3308);
                              KILL(zTransaction)(&z3zE3309);
                              KILL(zTransaction)(&z3zE3310);
                              KILL(zTransaction)(&z3zE3311);
                              KILL(zTransaction)(&z3zE3312);
                              KILL(zTransaction)(&z3zE3313);
                              KILL(zTransaction)(&z3zE3314);
                              goto end_block_exception_3410;
                            }
                          }
                          struct ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9 z2zE3295;
                          CREATE(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3295);
                          {
                            zdecode_access_list(&z2zE3295, zal_f);
                            if (have_exception) {
                              KILL(zTransaction)(&z3zE3205);
                              KILL(zTransaction)(&z3zE3304);
                              KILL(zTransaction)(&z3zE3305);
                              KILL(zTransaction)(&z3zE3306);
                              KILL(zTransaction)(&z3zE3307);
                              KILL(zTransaction)(&z3zE3308);
                              KILL(zTransaction)(&z3zE3309);
                              KILL(zTransaction)(&z3zE3310);
                              KILL(zTransaction)(&z3zE3311);
                              KILL(zTransaction)(&z3zE3312);
                              KILL(zTransaction)(&z3zE3313);
                              KILL(zTransaction)(&z3zE3314);
                              KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3295);
                              goto end_block_exception_3410;
                            }
                          }
                          struct zTransaction z3zE3316;
                          CREATE(zTransaction)(&z3zE3316);
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
                            sail_u256 z2zE3296;
                            {
                              z2zE3296 = zrlp_ref_uint_word(z3zE3357);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE3205);
                                KILL(zTransaction)(&z3zE3304);
                                KILL(zTransaction)(&z3zE3305);
                                KILL(zTransaction)(&z3zE3306);
                                KILL(zTransaction)(&z3zE3307);
                                KILL(zTransaction)(&z3zE3308);
                                KILL(zTransaction)(&z3zE3309);
                                KILL(zTransaction)(&z3zE3310);
                                KILL(zTransaction)(&z3zE3311);
                                KILL(zTransaction)(&z3zE3312);
                                KILL(zTransaction)(&z3zE3313);
                                KILL(zTransaction)(&z3zE3314);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3295);
                                KILL(zTransaction)(&z3zE3316);
                                KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                goto end_block_exception_3410;
                              }
                            }
                            uint64_t z2zE3297;
                            {
                              z2zE3297 = zrlp_ref_chain_identifier(zchain_f);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE3205);
                                KILL(zTransaction)(&z3zE3304);
                                KILL(zTransaction)(&z3zE3305);
                                KILL(zTransaction)(&z3zE3306);
                                KILL(zTransaction)(&z3zE3307);
                                KILL(zTransaction)(&z3zE3308);
                                KILL(zTransaction)(&z3zE3309);
                                KILL(zTransaction)(&z3zE3310);
                                KILL(zTransaction)(&z3zE3311);
                                KILL(zTransaction)(&z3zE3312);
                                KILL(zTransaction)(&z3zE3313);
                                KILL(zTransaction)(&z3zE3314);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3295);
                                KILL(zTransaction)(&z3zE3316);
                                KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                goto end_block_exception_3410;
                              }
                            }
                            uint64_t z2zE3298;
                            {
                              z2zE3298 = zrlp_ref_gas(z3zE3359, zfork);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE3205);
                                KILL(zTransaction)(&z3zE3304);
                                KILL(zTransaction)(&z3zE3305);
                                KILL(zTransaction)(&z3zE3306);
                                KILL(zTransaction)(&z3zE3307);
                                KILL(zTransaction)(&z3zE3308);
                                KILL(zTransaction)(&z3zE3309);
                                KILL(zTransaction)(&z3zE3310);
                                KILL(zTransaction)(&z3zE3311);
                                KILL(zTransaction)(&z3zE3312);
                                KILL(zTransaction)(&z3zE3313);
                                KILL(zTransaction)(&z3zE3314);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3295);
                                KILL(zTransaction)(&z3zE3316);
                                KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                goto end_block_exception_3410;
                              }
                            }
                            bool z2zE3300;
                            {
                              uint64_t z2zE3299;
                              z2zE3299 = z3zE3360.zcontent_len;
                              z2zE3300 = (z2zE3299 == UINT64_C(0));
                            }
                            sail_fixed_bytes_20 z2zE3302;
                            {
                              sail_u256 z2zE3301;
                              {
                                z2zE3301 = zrlp_ref_word(z3zE3360);
                                if (have_exception) {
                                  KILL(zTransaction)(&z3zE3205);
                                  KILL(zTransaction)(&z3zE3304);
                                  KILL(zTransaction)(&z3zE3305);
                                  KILL(zTransaction)(&z3zE3306);
                                  KILL(zTransaction)(&z3zE3307);
                                  KILL(zTransaction)(&z3zE3308);
                                  KILL(zTransaction)(&z3zE3309);
                                  KILL(zTransaction)(&z3zE3310);
                                  KILL(zTransaction)(&z3zE3311);
                                  KILL(zTransaction)(&z3zE3312);
                                  KILL(zTransaction)(&z3zE3313);
                                  KILL(zTransaction)(&z3zE3314);
                                  KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3295);
                                  KILL(zTransaction)(&z3zE3316);
                                  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                  KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                  goto end_block_exception_3410;
                                }
                              }
                              z2zE3302 = zword_to_address(z2zE3301);
                            }
                            sail_u256 z2zE3303;
                            {
                              z2zE3303 = zrlp_ref_uint_word(z3zE3361);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE3205);
                                KILL(zTransaction)(&z3zE3304);
                                KILL(zTransaction)(&z3zE3305);
                                KILL(zTransaction)(&z3zE3306);
                                KILL(zTransaction)(&z3zE3307);
                                KILL(zTransaction)(&z3zE3308);
                                KILL(zTransaction)(&z3zE3309);
                                KILL(zTransaction)(&z3zE3310);
                                KILL(zTransaction)(&z3zE3311);
                                KILL(zTransaction)(&z3zE3312);
                                KILL(zTransaction)(&z3zE3313);
                                KILL(zTransaction)(&z3zE3314);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3295);
                                KILL(zTransaction)(&z3zE3316);
                                KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                goto end_block_exception_3410;
                              }
                            }
                            struct zByteSliceFields z2zE3304;
                            {
                              z2zE3304 = ztx_input_span(z3zE3362);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE3205);
                                KILL(zTransaction)(&z3zE3304);
                                KILL(zTransaction)(&z3zE3305);
                                KILL(zTransaction)(&z3zE3306);
                                KILL(zTransaction)(&z3zE3307);
                                KILL(zTransaction)(&z3zE3308);
                                KILL(zTransaction)(&z3zE3309);
                                KILL(zTransaction)(&z3zE3310);
                                KILL(zTransaction)(&z3zE3311);
                                KILL(zTransaction)(&z3zE3312);
                                KILL(zTransaction)(&z3zE3313);
                                KILL(zTransaction)(&z3zE3314);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3295);
                                KILL(zTransaction)(&z3zE3316);
                                KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                goto end_block_exception_3410;
                              }
                            }
                            sail_fixed_bytes_32 z2zE3306;
                            {
                              struct zByteSliceFields z2zE3305;
                              {
                                z2zE3305 = ztx_sig_span(zchain_f, z3zE3363);
                                if (have_exception) {
                                  KILL(zTransaction)(&z3zE3205);
                                  KILL(zTransaction)(&z3zE3304);
                                  KILL(zTransaction)(&z3zE3305);
                                  KILL(zTransaction)(&z3zE3306);
                                  KILL(zTransaction)(&z3zE3307);
                                  KILL(zTransaction)(&z3zE3308);
                                  KILL(zTransaction)(&z3zE3309);
                                  KILL(zTransaction)(&z3zE3310);
                                  KILL(zTransaction)(&z3zE3311);
                                  KILL(zTransaction)(&z3zE3312);
                                  KILL(zTransaction)(&z3zE3313);
                                  KILL(zTransaction)(&z3zE3314);
                                  KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3295);
                                  KILL(zTransaction)(&z3zE3316);
                                  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                  KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                  goto end_block_exception_3410;
                                }
                              }
                              {
                                z2zE3306 = ztx_signing_hash(zAccessListTx, z2zE3305, z3zE3366);
                                if (have_exception) {
                                  KILL(zTransaction)(&z3zE3205);
                                  KILL(zTransaction)(&z3zE3304);
                                  KILL(zTransaction)(&z3zE3305);
                                  KILL(zTransaction)(&z3zE3306);
                                  KILL(zTransaction)(&z3zE3307);
                                  KILL(zTransaction)(&z3zE3308);
                                  KILL(zTransaction)(&z3zE3309);
                                  KILL(zTransaction)(&z3zE3310);
                                  KILL(zTransaction)(&z3zE3311);
                                  KILL(zTransaction)(&z3zE3312);
                                  KILL(zTransaction)(&z3zE3313);
                                  KILL(zTransaction)(&z3zE3314);
                                  KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3295);
                                  KILL(zTransaction)(&z3zE3316);
                                  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                  KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                  goto end_block_exception_3410;
                                }
                              }
                            }
                            sail_u256 z2zE3307;
                            {
                              z2zE3307 = zrlp_ref_uint_word(z3zE3364);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE3205);
                                KILL(zTransaction)(&z3zE3304);
                                KILL(zTransaction)(&z3zE3305);
                                KILL(zTransaction)(&z3zE3306);
                                KILL(zTransaction)(&z3zE3307);
                                KILL(zTransaction)(&z3zE3308);
                                KILL(zTransaction)(&z3zE3309);
                                KILL(zTransaction)(&z3zE3310);
                                KILL(zTransaction)(&z3zE3311);
                                KILL(zTransaction)(&z3zE3312);
                                KILL(zTransaction)(&z3zE3313);
                                KILL(zTransaction)(&z3zE3314);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3295);
                                KILL(zTransaction)(&z3zE3316);
                                KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                goto end_block_exception_3410;
                              }
                            }
                            sail_u256 z2zE3308;
                            {
                              z2zE3308 = zrlp_ref_uint_word(z3zE3365);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE3205);
                                KILL(zTransaction)(&z3zE3304);
                                KILL(zTransaction)(&z3zE3305);
                                KILL(zTransaction)(&z3zE3306);
                                KILL(zTransaction)(&z3zE3307);
                                KILL(zTransaction)(&z3zE3308);
                                KILL(zTransaction)(&z3zE3309);
                                KILL(zTransaction)(&z3zE3310);
                                KILL(zTransaction)(&z3zE3311);
                                KILL(zTransaction)(&z3zE3312);
                                KILL(zTransaction)(&z3zE3313);
                                KILL(zTransaction)(&z3zE3314);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3295);
                                KILL(zTransaction)(&z3zE3316);
                                KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                goto end_block_exception_3410;
                              }
                            }
                            z3zE3316.zaccess_list_address_count = zal_addr_count;
                            COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE3316)->zaccess_list_addresses), zal_addrs);
                            z3zE3316.zaccess_list_slot_count = zal_slot_count;
                            COPY(zz5listz8z5structz0zzStorageKeyz9)(&((&z3zE3316)->zaccess_list_slots), zal_slots);
                            z3zE3316.zauthorizzation_count = UINT64_C(0);
                            zz5listz8z5structz0zzAuthorizzzzationz9 z3zE3318;
                            CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&z3zE3318);
                            COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&((&z3zE3316)->zauthorizzations), z3zE3318);
                            KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z3zE3318);
                            z3zE3316.zblob_hashes = zEMPTY_BLOB_HASHES;
                            z3zE3316.zchain_id = z2zE3297;
                            z3zE3316.zgas_limit = z2zE3298;
                            z3zE3316.zinput_src = z2zE3304;
                            z3zE3316.zis_create = z2zE3300;
                            z3zE3316.zmax_blob_fee = zZERO_WORD;
                            z3zE3316.zmax_fee = z3zE3367;
                            z3zE3316.zmax_priority_fee = z3zE3367;
                            z3zE3316.znonce = z2zE3296;
                            z3zE3316.zpubkey = zpubkey;
                            z3zE3316.zraw = ztx;
                            z3zE3316.zrecipient = z2zE3302;
                            z3zE3316.zsender = zsender;
                            z3zE3316.zsig_r = z2zE3307;
                            z3zE3316.zsig_s = z2zE3308;
                            z3zE3316.zsig_v = z3zE3366;
                            z3zE3316.zsigning_hash = z2zE3306;
                            z3zE3316.ztx_type = zAccessListTx;
                            z3zE3316.zvalue = z2zE3303;
                            KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                            KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                            goto finish_match_3388;
                          }
                        case_3389: ;
                          sail_match_failure("rlp_decode_tx");
                        finish_match_3388: ;
                          COPY(zTransaction)(&z3zE3314, z3zE3316);
                          KILL(zTransaction)(&z3zE3316);
                          KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3295);
                          goto finish_match_3386;
                        }
                      case_3387: ;
                        sail_match_failure("rlp_decode_tx");
                      finish_match_3386: ;
                        COPY(zTransaction)(&z3zE3313, z3zE3314);
                        KILL(zTransaction)(&z3zE3314);
                        goto finish_match_3384;
                      }
                    case_3385: ;
                      sail_match_failure("rlp_decode_tx");
                    finish_match_3384: ;
                      COPY(zTransaction)(&z3zE3312, z3zE3313);
                      KILL(zTransaction)(&z3zE3313);
                      goto finish_match_3382;
                    }
                  case_3383: ;
                    sail_match_failure("rlp_decode_tx");
                  finish_match_3382: ;
                    COPY(zTransaction)(&z3zE3311, z3zE3312);
                    KILL(zTransaction)(&z3zE3312);
                    goto finish_match_3380;
                  }
                case_3381: ;
                  sail_match_failure("rlp_decode_tx");
                finish_match_3380: ;
                  COPY(zTransaction)(&z3zE3310, z3zE3311);
                  KILL(zTransaction)(&z3zE3311);
                  goto finish_match_3378;
                }
              case_3379: ;
                sail_match_failure("rlp_decode_tx");
              finish_match_3378: ;
                COPY(zTransaction)(&z3zE3309, z3zE3310);
                KILL(zTransaction)(&z3zE3310);
                goto finish_match_3376;
              }
            case_3377: ;
              sail_match_failure("rlp_decode_tx");
            finish_match_3376: ;
              COPY(zTransaction)(&z3zE3308, z3zE3309);
              KILL(zTransaction)(&z3zE3309);
              goto finish_match_3374;
            }
          case_3375: ;
            sail_match_failure("rlp_decode_tx");
          finish_match_3374: ;
            COPY(zTransaction)(&z3zE3307, z3zE3308);
            KILL(zTransaction)(&z3zE3308);
            goto finish_match_3372;
          }
        case_3373: ;
          sail_match_failure("rlp_decode_tx");
        finish_match_3372: ;
          COPY(zTransaction)(&z3zE3306, z3zE3307);
          KILL(zTransaction)(&z3zE3307);
          goto finish_match_3370;
        }
      case_3371: ;
        sail_match_failure("rlp_decode_tx");
      finish_match_3370: ;
        COPY(zTransaction)(&z3zE3305, z3zE3306);
        KILL(zTransaction)(&z3zE3306);
        goto finish_match_3368;
      }
    case_3369: ;
      sail_match_failure("rlp_decode_tx");
    finish_match_3368: ;
      COPY(zTransaction)(&z3zE3304, z3zE3305);
      KILL(zTransaction)(&z3zE3305);
      goto finish_match_3366;
    }
  case_3367: ;
    sail_match_failure("rlp_decode_tx");
  finish_match_3366: ;
    COPY(zTransaction)(&z3zE3205, z3zE3304);
    KILL(zTransaction)(&z3zE3304);
    goto finish_match_3274;
  }
case_3365: ;
  {
    uint64_t z3zE3368;
    z3zE3368 = zttype;
    bool z3zE3303;
    z3zE3303 = (z3zE3368 == UINT64_C(0x02));
    if (!(z3zE3303)) {  goto case_3338;  }
    struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3309;
    {
      z2zE3309 = zrlp_cursor_pop(zfields);
      if (have_exception) {
        KILL(zTransaction)(&z3zE3205);
        goto end_block_exception_3410;
      }
    }
    struct zTransaction z3zE3274;
    CREATE(zTransaction)(&z3zE3274);
    {
      struct zRlpFieldRefFields z3zE3369;
      z3zE3369 = z2zE3309.ztup0;
      struct zRlpCursorFields z1zE29;
      z1zE29 = z2zE3309.ztup1;
      struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3310;
      {
        z2zE3310 = zrlp_cursor_pop(z1zE29);
        if (have_exception) {
          KILL(zTransaction)(&z3zE3205);
          KILL(zTransaction)(&z3zE3274);
          goto end_block_exception_3410;
        }
      }
      struct zTransaction z3zE3275;
      CREATE(zTransaction)(&z3zE3275);
      {
        struct zRlpFieldRefFields z3zE3370;
        z3zE3370 = z2zE3310.ztup0;
        struct zRlpCursorFields z1zE30;
        z1zE30 = z2zE3310.ztup1;
        struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3311;
        {
          z2zE3311 = zrlp_cursor_pop(z1zE30);
          if (have_exception) {
            KILL(zTransaction)(&z3zE3205);
            KILL(zTransaction)(&z3zE3274);
            KILL(zTransaction)(&z3zE3275);
            goto end_block_exception_3410;
          }
        }
        struct zTransaction z3zE3276;
        CREATE(zTransaction)(&z3zE3276);
        {
          struct zRlpFieldRefFields zmp_f;
          zmp_f = z2zE3311.ztup0;
          struct zRlpCursorFields z1zE31;
          z1zE31 = z2zE3311.ztup1;
          struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3312;
          {
            z2zE3312 = zrlp_cursor_pop(z1zE31);
            if (have_exception) {
              KILL(zTransaction)(&z3zE3205);
              KILL(zTransaction)(&z3zE3274);
              KILL(zTransaction)(&z3zE3275);
              KILL(zTransaction)(&z3zE3276);
              goto end_block_exception_3410;
            }
          }
          struct zTransaction z3zE3277;
          CREATE(zTransaction)(&z3zE3277);
          {
            struct zRlpFieldRefFields zmf_f;
            zmf_f = z2zE3312.ztup0;
            struct zRlpCursorFields z1zE32;
            z1zE32 = z2zE3312.ztup1;
            struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3313;
            {
              z2zE3313 = zrlp_cursor_pop(z1zE32);
              if (have_exception) {
                KILL(zTransaction)(&z3zE3205);
                KILL(zTransaction)(&z3zE3274);
                KILL(zTransaction)(&z3zE3275);
                KILL(zTransaction)(&z3zE3276);
                KILL(zTransaction)(&z3zE3277);
                goto end_block_exception_3410;
              }
            }
            struct zTransaction z3zE3278;
            CREATE(zTransaction)(&z3zE3278);
            {
              struct zRlpFieldRefFields z3zE3371;
              z3zE3371 = z2zE3313.ztup0;
              struct zRlpCursorFields z1zE33;
              z1zE33 = z2zE3313.ztup1;
              struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3314;
              {
                z2zE3314 = zrlp_cursor_pop(z1zE33);
                if (have_exception) {
                  KILL(zTransaction)(&z3zE3205);
                  KILL(zTransaction)(&z3zE3274);
                  KILL(zTransaction)(&z3zE3275);
                  KILL(zTransaction)(&z3zE3276);
                  KILL(zTransaction)(&z3zE3277);
                  KILL(zTransaction)(&z3zE3278);
                  goto end_block_exception_3410;
                }
              }
              struct zTransaction z3zE3279;
              CREATE(zTransaction)(&z3zE3279);
              {
                struct zRlpFieldRefFields z3zE3372;
                z3zE3372 = z2zE3314.ztup0;
                struct zRlpCursorFields z1zE34;
                z1zE34 = z2zE3314.ztup1;
                struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3315;
                {
                  z2zE3315 = zrlp_cursor_pop(z1zE34);
                  if (have_exception) {
                    KILL(zTransaction)(&z3zE3205);
                    KILL(zTransaction)(&z3zE3274);
                    KILL(zTransaction)(&z3zE3275);
                    KILL(zTransaction)(&z3zE3276);
                    KILL(zTransaction)(&z3zE3277);
                    KILL(zTransaction)(&z3zE3278);
                    KILL(zTransaction)(&z3zE3279);
                    goto end_block_exception_3410;
                  }
                }
                struct zTransaction z3zE3280;
                CREATE(zTransaction)(&z3zE3280);
                {
                  struct zRlpFieldRefFields z3zE3373;
                  z3zE3373 = z2zE3315.ztup0;
                  struct zRlpCursorFields z1zE35;
                  z1zE35 = z2zE3315.ztup1;
                  struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3316;
                  {
                    z2zE3316 = zrlp_cursor_pop(z1zE35);
                    if (have_exception) {
                      KILL(zTransaction)(&z3zE3205);
                      KILL(zTransaction)(&z3zE3274);
                      KILL(zTransaction)(&z3zE3275);
                      KILL(zTransaction)(&z3zE3276);
                      KILL(zTransaction)(&z3zE3277);
                      KILL(zTransaction)(&z3zE3278);
                      KILL(zTransaction)(&z3zE3279);
                      KILL(zTransaction)(&z3zE3280);
                      goto end_block_exception_3410;
                    }
                  }
                  struct zTransaction z3zE3281;
                  CREATE(zTransaction)(&z3zE3281);
                  {
                    struct zRlpFieldRefFields z3zE3374;
                    z3zE3374 = z2zE3316.ztup0;
                    struct zRlpCursorFields z1zE36;
                    z1zE36 = z2zE3316.ztup1;
                    struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3317;
                    {
                      z2zE3317 = zrlp_cursor_pop(z1zE36);
                      if (have_exception) {
                        KILL(zTransaction)(&z3zE3205);
                        KILL(zTransaction)(&z3zE3274);
                        KILL(zTransaction)(&z3zE3275);
                        KILL(zTransaction)(&z3zE3276);
                        KILL(zTransaction)(&z3zE3277);
                        KILL(zTransaction)(&z3zE3278);
                        KILL(zTransaction)(&z3zE3279);
                        KILL(zTransaction)(&z3zE3280);
                        KILL(zTransaction)(&z3zE3281);
                        goto end_block_exception_3410;
                      }
                    }
                    struct zTransaction z3zE3282;
                    CREATE(zTransaction)(&z3zE3282);
                    {
                      struct zRlpFieldRefFields z3zE3375;
                      z3zE3375 = z2zE3317.ztup0;
                      struct zRlpCursorFields z1zE37;
                      z1zE37 = z2zE3317.ztup1;
                      struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3318;
                      {
                        z2zE3318 = zrlp_cursor_pop(z1zE37);
                        if (have_exception) {
                          KILL(zTransaction)(&z3zE3205);
                          KILL(zTransaction)(&z3zE3274);
                          KILL(zTransaction)(&z3zE3275);
                          KILL(zTransaction)(&z3zE3276);
                          KILL(zTransaction)(&z3zE3277);
                          KILL(zTransaction)(&z3zE3278);
                          KILL(zTransaction)(&z3zE3279);
                          KILL(zTransaction)(&z3zE3280);
                          KILL(zTransaction)(&z3zE3281);
                          KILL(zTransaction)(&z3zE3282);
                          goto end_block_exception_3410;
                        }
                      }
                      struct zTransaction z3zE3283;
                      CREATE(zTransaction)(&z3zE3283);
                      {
                        struct zRlpFieldRefFields z3zE3376;
                        z3zE3376 = z2zE3318.ztup0;
                        struct zRlpCursorFields z1zE38;
                        z1zE38 = z2zE3318.ztup1;
                        struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3319;
                        {
                          z2zE3319 = zrlp_cursor_pop(z1zE38);
                          if (have_exception) {
                            KILL(zTransaction)(&z3zE3205);
                            KILL(zTransaction)(&z3zE3274);
                            KILL(zTransaction)(&z3zE3275);
                            KILL(zTransaction)(&z3zE3276);
                            KILL(zTransaction)(&z3zE3277);
                            KILL(zTransaction)(&z3zE3278);
                            KILL(zTransaction)(&z3zE3279);
                            KILL(zTransaction)(&z3zE3280);
                            KILL(zTransaction)(&z3zE3281);
                            KILL(zTransaction)(&z3zE3282);
                            KILL(zTransaction)(&z3zE3283);
                            goto end_block_exception_3410;
                          }
                        }
                        struct zTransaction z3zE3284;
                        CREATE(zTransaction)(&z3zE3284);
                        {
                          struct zRlpFieldRefFields z3zE3377;
                          z3zE3377 = z2zE3319.ztup0;
                          struct zRlpCursorFields z1zE39;
                          z1zE39 = z2zE3319.ztup1;
                          struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3320;
                          {
                            z2zE3320 = zrlp_cursor_pop(z1zE39);
                            if (have_exception) {
                              KILL(zTransaction)(&z3zE3205);
                              KILL(zTransaction)(&z3zE3274);
                              KILL(zTransaction)(&z3zE3275);
                              KILL(zTransaction)(&z3zE3276);
                              KILL(zTransaction)(&z3zE3277);
                              KILL(zTransaction)(&z3zE3278);
                              KILL(zTransaction)(&z3zE3279);
                              KILL(zTransaction)(&z3zE3280);
                              KILL(zTransaction)(&z3zE3281);
                              KILL(zTransaction)(&z3zE3282);
                              KILL(zTransaction)(&z3zE3283);
                              KILL(zTransaction)(&z3zE3284);
                              goto end_block_exception_3410;
                            }
                          }
                          struct zTransaction z3zE3285;
                          CREATE(zTransaction)(&z3zE3285);
                          {
                            struct zRlpFieldRefFields z3zE3378;
                            z3zE3378 = z2zE3320.ztup0;
                            struct zRlpCursorFields z1zE40;
                            z1zE40 = z2zE3320.ztup1;
                            unit z3zE3286;
                            {
                              z3zE3286 = zrlp_cursor_expect_end(z1zE40);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE3205);
                                KILL(zTransaction)(&z3zE3274);
                                KILL(zTransaction)(&z3zE3275);
                                KILL(zTransaction)(&z3zE3276);
                                KILL(zTransaction)(&z3zE3277);
                                KILL(zTransaction)(&z3zE3278);
                                KILL(zTransaction)(&z3zE3279);
                                KILL(zTransaction)(&z3zE3280);
                                KILL(zTransaction)(&z3zE3281);
                                KILL(zTransaction)(&z3zE3282);
                                KILL(zTransaction)(&z3zE3283);
                                KILL(zTransaction)(&z3zE3284);
                                KILL(zTransaction)(&z3zE3285);
                                goto end_block_exception_3410;
                              }
                            }
                            sail_u256 z3zE3379;
                            {
                              z3zE3379 = zrlp_ref_word(z3zE3376);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE3205);
                                KILL(zTransaction)(&z3zE3274);
                                KILL(zTransaction)(&z3zE3275);
                                KILL(zTransaction)(&z3zE3276);
                                KILL(zTransaction)(&z3zE3277);
                                KILL(zTransaction)(&z3zE3278);
                                KILL(zTransaction)(&z3zE3279);
                                KILL(zTransaction)(&z3zE3280);
                                KILL(zTransaction)(&z3zE3281);
                                KILL(zTransaction)(&z3zE3282);
                                KILL(zTransaction)(&z3zE3283);
                                KILL(zTransaction)(&z3zE3284);
                                KILL(zTransaction)(&z3zE3285);
                                goto end_block_exception_3410;
                              }
                            }
                            struct ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9 z2zE3321;
                            CREATE(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3321);
                            {
                              zdecode_access_list(&z2zE3321, z3zE3375);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE3205);
                                KILL(zTransaction)(&z3zE3274);
                                KILL(zTransaction)(&z3zE3275);
                                KILL(zTransaction)(&z3zE3276);
                                KILL(zTransaction)(&z3zE3277);
                                KILL(zTransaction)(&z3zE3278);
                                KILL(zTransaction)(&z3zE3279);
                                KILL(zTransaction)(&z3zE3280);
                                KILL(zTransaction)(&z3zE3281);
                                KILL(zTransaction)(&z3zE3282);
                                KILL(zTransaction)(&z3zE3283);
                                KILL(zTransaction)(&z3zE3284);
                                KILL(zTransaction)(&z3zE3285);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3321);
                                goto end_block_exception_3410;
                              }
                            }
                            struct zTransaction z3zE3287;
                            CREATE(zTransaction)(&z3zE3287);
                            {
                              zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z3zE3380;
                              CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3380);
                              COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3380, z2zE3321.ztup0);
                              zz5listz8z5structz0zzStorageKeyz9 z3zE3381;
                              CREATE(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3381);
                              COPY(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3381, z2zE3321.ztup1);
                              uint64_t z3zE3382;
                              z3zE3382 = z2zE3321.ztup2;
                              uint64_t z3zE3383;
                              z3zE3383 = z2zE3321.ztup3;
                              sail_u256 z2zE3322;
                              {
                                z2zE3322 = zrlp_ref_uint_word(z3zE3370);
                                if (have_exception) {
                                  KILL(zTransaction)(&z3zE3205);
                                  KILL(zTransaction)(&z3zE3274);
                                  KILL(zTransaction)(&z3zE3275);
                                  KILL(zTransaction)(&z3zE3276);
                                  KILL(zTransaction)(&z3zE3277);
                                  KILL(zTransaction)(&z3zE3278);
                                  KILL(zTransaction)(&z3zE3279);
                                  KILL(zTransaction)(&z3zE3280);
                                  KILL(zTransaction)(&z3zE3281);
                                  KILL(zTransaction)(&z3zE3282);
                                  KILL(zTransaction)(&z3zE3283);
                                  KILL(zTransaction)(&z3zE3284);
                                  KILL(zTransaction)(&z3zE3285);
                                  KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3321);
                                  KILL(zTransaction)(&z3zE3287);
                                  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3380);
                                  KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3381);
                                  goto end_block_exception_3410;
                                }
                              }
                              uint64_t z2zE3323;
                              {
                                z2zE3323 = zrlp_ref_chain_identifier(z3zE3369);
                                if (have_exception) {
                                  KILL(zTransaction)(&z3zE3205);
                                  KILL(zTransaction)(&z3zE3274);
                                  KILL(zTransaction)(&z3zE3275);
                                  KILL(zTransaction)(&z3zE3276);
                                  KILL(zTransaction)(&z3zE3277);
                                  KILL(zTransaction)(&z3zE3278);
                                  KILL(zTransaction)(&z3zE3279);
                                  KILL(zTransaction)(&z3zE3280);
                                  KILL(zTransaction)(&z3zE3281);
                                  KILL(zTransaction)(&z3zE3282);
                                  KILL(zTransaction)(&z3zE3283);
                                  KILL(zTransaction)(&z3zE3284);
                                  KILL(zTransaction)(&z3zE3285);
                                  KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3321);
                                  KILL(zTransaction)(&z3zE3287);
                                  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3380);
                                  KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3381);
                                  goto end_block_exception_3410;
                                }
                              }
                              uint64_t z2zE3324;
                              {
                                z2zE3324 = zrlp_ref_gas(z3zE3371, zfork);
                                if (have_exception) {
                                  KILL(zTransaction)(&z3zE3205);
                                  KILL(zTransaction)(&z3zE3274);
                                  KILL(zTransaction)(&z3zE3275);
                                  KILL(zTransaction)(&z3zE3276);
                                  KILL(zTransaction)(&z3zE3277);
                                  KILL(zTransaction)(&z3zE3278);
                                  KILL(zTransaction)(&z3zE3279);
                                  KILL(zTransaction)(&z3zE3280);
                                  KILL(zTransaction)(&z3zE3281);
                                  KILL(zTransaction)(&z3zE3282);
                                  KILL(zTransaction)(&z3zE3283);
                                  KILL(zTransaction)(&z3zE3284);
                                  KILL(zTransaction)(&z3zE3285);
                                  KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3321);
                                  KILL(zTransaction)(&z3zE3287);
                                  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3380);
                                  KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3381);
                                  goto end_block_exception_3410;
                                }
                              }
                              bool z2zE3326;
                              {
                                uint64_t z2zE3325;
                                z2zE3325 = z3zE3372.zcontent_len;
                                z2zE3326 = (z2zE3325 == UINT64_C(0));
                              }
                              sail_fixed_bytes_20 z2zE3328;
                              {
                                sail_u256 z2zE3327;
                                {
                                  z2zE3327 = zrlp_ref_word(z3zE3372);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE3205);
                                    KILL(zTransaction)(&z3zE3274);
                                    KILL(zTransaction)(&z3zE3275);
                                    KILL(zTransaction)(&z3zE3276);
                                    KILL(zTransaction)(&z3zE3277);
                                    KILL(zTransaction)(&z3zE3278);
                                    KILL(zTransaction)(&z3zE3279);
                                    KILL(zTransaction)(&z3zE3280);
                                    KILL(zTransaction)(&z3zE3281);
                                    KILL(zTransaction)(&z3zE3282);
                                    KILL(zTransaction)(&z3zE3283);
                                    KILL(zTransaction)(&z3zE3284);
                                    KILL(zTransaction)(&z3zE3285);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3321);
                                    KILL(zTransaction)(&z3zE3287);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3380);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3381);
                                    goto end_block_exception_3410;
                                  }
                                }
                                z2zE3328 = zword_to_address(z2zE3327);
                              }
                              sail_u256 z2zE3329;
                              {
                                z2zE3329 = zrlp_ref_uint_word(z3zE3373);
                                if (have_exception) {
                                  KILL(zTransaction)(&z3zE3205);
                                  KILL(zTransaction)(&z3zE3274);
                                  KILL(zTransaction)(&z3zE3275);
                                  KILL(zTransaction)(&z3zE3276);
                                  KILL(zTransaction)(&z3zE3277);
                                  KILL(zTransaction)(&z3zE3278);
                                  KILL(zTransaction)(&z3zE3279);
                                  KILL(zTransaction)(&z3zE3280);
                                  KILL(zTransaction)(&z3zE3281);
                                  KILL(zTransaction)(&z3zE3282);
                                  KILL(zTransaction)(&z3zE3283);
                                  KILL(zTransaction)(&z3zE3284);
                                  KILL(zTransaction)(&z3zE3285);
                                  KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3321);
                                  KILL(zTransaction)(&z3zE3287);
                                  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3380);
                                  KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3381);
                                  goto end_block_exception_3410;
                                }
                              }
                              struct zByteSliceFields z2zE3330;
                              {
                                z2zE3330 = ztx_input_span(z3zE3374);
                                if (have_exception) {
                                  KILL(zTransaction)(&z3zE3205);
                                  KILL(zTransaction)(&z3zE3274);
                                  KILL(zTransaction)(&z3zE3275);
                                  KILL(zTransaction)(&z3zE3276);
                                  KILL(zTransaction)(&z3zE3277);
                                  KILL(zTransaction)(&z3zE3278);
                                  KILL(zTransaction)(&z3zE3279);
                                  KILL(zTransaction)(&z3zE3280);
                                  KILL(zTransaction)(&z3zE3281);
                                  KILL(zTransaction)(&z3zE3282);
                                  KILL(zTransaction)(&z3zE3283);
                                  KILL(zTransaction)(&z3zE3284);
                                  KILL(zTransaction)(&z3zE3285);
                                  KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3321);
                                  KILL(zTransaction)(&z3zE3287);
                                  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3380);
                                  KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3381);
                                  goto end_block_exception_3410;
                                }
                              }
                              sail_u256 z2zE3331;
                              {
                                z2zE3331 = zrlp_ref_uint_word(zmf_f);
                                if (have_exception) {
                                  KILL(zTransaction)(&z3zE3205);
                                  KILL(zTransaction)(&z3zE3274);
                                  KILL(zTransaction)(&z3zE3275);
                                  KILL(zTransaction)(&z3zE3276);
                                  KILL(zTransaction)(&z3zE3277);
                                  KILL(zTransaction)(&z3zE3278);
                                  KILL(zTransaction)(&z3zE3279);
                                  KILL(zTransaction)(&z3zE3280);
                                  KILL(zTransaction)(&z3zE3281);
                                  KILL(zTransaction)(&z3zE3282);
                                  KILL(zTransaction)(&z3zE3283);
                                  KILL(zTransaction)(&z3zE3284);
                                  KILL(zTransaction)(&z3zE3285);
                                  KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3321);
                                  KILL(zTransaction)(&z3zE3287);
                                  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3380);
                                  KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3381);
                                  goto end_block_exception_3410;
                                }
                              }
                              sail_u256 z2zE3332;
                              {
                                z2zE3332 = zrlp_ref_uint_word(zmp_f);
                                if (have_exception) {
                                  KILL(zTransaction)(&z3zE3205);
                                  KILL(zTransaction)(&z3zE3274);
                                  KILL(zTransaction)(&z3zE3275);
                                  KILL(zTransaction)(&z3zE3276);
                                  KILL(zTransaction)(&z3zE3277);
                                  KILL(zTransaction)(&z3zE3278);
                                  KILL(zTransaction)(&z3zE3279);
                                  KILL(zTransaction)(&z3zE3280);
                                  KILL(zTransaction)(&z3zE3281);
                                  KILL(zTransaction)(&z3zE3282);
                                  KILL(zTransaction)(&z3zE3283);
                                  KILL(zTransaction)(&z3zE3284);
                                  KILL(zTransaction)(&z3zE3285);
                                  KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3321);
                                  KILL(zTransaction)(&z3zE3287);
                                  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3380);
                                  KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3381);
                                  goto end_block_exception_3410;
                                }
                              }
                              sail_fixed_bytes_32 z2zE3334;
                              {
                                struct zByteSliceFields z2zE3333;
                                {
                                  z2zE3333 = ztx_sig_span(z3zE3369, z3zE3376);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE3205);
                                    KILL(zTransaction)(&z3zE3274);
                                    KILL(zTransaction)(&z3zE3275);
                                    KILL(zTransaction)(&z3zE3276);
                                    KILL(zTransaction)(&z3zE3277);
                                    KILL(zTransaction)(&z3zE3278);
                                    KILL(zTransaction)(&z3zE3279);
                                    KILL(zTransaction)(&z3zE3280);
                                    KILL(zTransaction)(&z3zE3281);
                                    KILL(zTransaction)(&z3zE3282);
                                    KILL(zTransaction)(&z3zE3283);
                                    KILL(zTransaction)(&z3zE3284);
                                    KILL(zTransaction)(&z3zE3285);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3321);
                                    KILL(zTransaction)(&z3zE3287);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3380);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3381);
                                    goto end_block_exception_3410;
                                  }
                                }
                                {
                                  z2zE3334 = ztx_signing_hash(zFeeMarketTx, z2zE3333, z3zE3379);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE3205);
                                    KILL(zTransaction)(&z3zE3274);
                                    KILL(zTransaction)(&z3zE3275);
                                    KILL(zTransaction)(&z3zE3276);
                                    KILL(zTransaction)(&z3zE3277);
                                    KILL(zTransaction)(&z3zE3278);
                                    KILL(zTransaction)(&z3zE3279);
                                    KILL(zTransaction)(&z3zE3280);
                                    KILL(zTransaction)(&z3zE3281);
                                    KILL(zTransaction)(&z3zE3282);
                                    KILL(zTransaction)(&z3zE3283);
                                    KILL(zTransaction)(&z3zE3284);
                                    KILL(zTransaction)(&z3zE3285);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3321);
                                    KILL(zTransaction)(&z3zE3287);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3380);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3381);
                                    goto end_block_exception_3410;
                                  }
                                }
                              }
                              sail_u256 z2zE3335;
                              {
                                z2zE3335 = zrlp_ref_uint_word(z3zE3377);
                                if (have_exception) {
                                  KILL(zTransaction)(&z3zE3205);
                                  KILL(zTransaction)(&z3zE3274);
                                  KILL(zTransaction)(&z3zE3275);
                                  KILL(zTransaction)(&z3zE3276);
                                  KILL(zTransaction)(&z3zE3277);
                                  KILL(zTransaction)(&z3zE3278);
                                  KILL(zTransaction)(&z3zE3279);
                                  KILL(zTransaction)(&z3zE3280);
                                  KILL(zTransaction)(&z3zE3281);
                                  KILL(zTransaction)(&z3zE3282);
                                  KILL(zTransaction)(&z3zE3283);
                                  KILL(zTransaction)(&z3zE3284);
                                  KILL(zTransaction)(&z3zE3285);
                                  KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3321);
                                  KILL(zTransaction)(&z3zE3287);
                                  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3380);
                                  KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3381);
                                  goto end_block_exception_3410;
                                }
                              }
                              sail_u256 z2zE3336;
                              {
                                z2zE3336 = zrlp_ref_uint_word(z3zE3378);
                                if (have_exception) {
                                  KILL(zTransaction)(&z3zE3205);
                                  KILL(zTransaction)(&z3zE3274);
                                  KILL(zTransaction)(&z3zE3275);
                                  KILL(zTransaction)(&z3zE3276);
                                  KILL(zTransaction)(&z3zE3277);
                                  KILL(zTransaction)(&z3zE3278);
                                  KILL(zTransaction)(&z3zE3279);
                                  KILL(zTransaction)(&z3zE3280);
                                  KILL(zTransaction)(&z3zE3281);
                                  KILL(zTransaction)(&z3zE3282);
                                  KILL(zTransaction)(&z3zE3283);
                                  KILL(zTransaction)(&z3zE3284);
                                  KILL(zTransaction)(&z3zE3285);
                                  KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3321);
                                  KILL(zTransaction)(&z3zE3287);
                                  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3380);
                                  KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3381);
                                  goto end_block_exception_3410;
                                }
                              }
                              z3zE3287.zaccess_list_address_count = z3zE3382;
                              COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE3287)->zaccess_list_addresses), z3zE3380);
                              z3zE3287.zaccess_list_slot_count = z3zE3383;
                              COPY(zz5listz8z5structz0zzStorageKeyz9)(&((&z3zE3287)->zaccess_list_slots), z3zE3381);
                              z3zE3287.zauthorizzation_count = UINT64_C(0);
                              zz5listz8z5structz0zzAuthorizzzzationz9 z3zE3289;
                              CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&z3zE3289);
                              COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&((&z3zE3287)->zauthorizzations), z3zE3289);
                              KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z3zE3289);
                              z3zE3287.zblob_hashes = zEMPTY_BLOB_HASHES;
                              z3zE3287.zchain_id = z2zE3323;
                              z3zE3287.zgas_limit = z2zE3324;
                              z3zE3287.zinput_src = z2zE3330;
                              z3zE3287.zis_create = z2zE3326;
                              z3zE3287.zmax_blob_fee = zZERO_WORD;
                              z3zE3287.zmax_fee = z2zE3331;
                              z3zE3287.zmax_priority_fee = z2zE3332;
                              z3zE3287.znonce = z2zE3322;
                              z3zE3287.zpubkey = zpubkey;
                              z3zE3287.zraw = ztx;
                              z3zE3287.zrecipient = z2zE3328;
                              z3zE3287.zsender = zsender;
                              z3zE3287.zsig_r = z2zE3335;
                              z3zE3287.zsig_s = z2zE3336;
                              z3zE3287.zsig_v = z3zE3379;
                              z3zE3287.zsigning_hash = z2zE3334;
                              z3zE3287.ztx_type = zFeeMarketTx;
                              z3zE3287.zvalue = z2zE3329;
                              KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3381);
                              KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3380);
                              goto finish_match_3363;
                            }
                          case_3364: ;
                            sail_match_failure("rlp_decode_tx");
                          finish_match_3363: ;
                            COPY(zTransaction)(&z3zE3285, z3zE3287);
                            KILL(zTransaction)(&z3zE3287);
                            KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3321);
                            goto finish_match_3361;
                          }
                        case_3362: ;
                          sail_match_failure("rlp_decode_tx");
                        finish_match_3361: ;
                          COPY(zTransaction)(&z3zE3284, z3zE3285);
                          KILL(zTransaction)(&z3zE3285);
                          goto finish_match_3359;
                        }
                      case_3360: ;
                        sail_match_failure("rlp_decode_tx");
                      finish_match_3359: ;
                        COPY(zTransaction)(&z3zE3283, z3zE3284);
                        KILL(zTransaction)(&z3zE3284);
                        goto finish_match_3357;
                      }
                    case_3358: ;
                      sail_match_failure("rlp_decode_tx");
                    finish_match_3357: ;
                      COPY(zTransaction)(&z3zE3282, z3zE3283);
                      KILL(zTransaction)(&z3zE3283);
                      goto finish_match_3355;
                    }
                  case_3356: ;
                    sail_match_failure("rlp_decode_tx");
                  finish_match_3355: ;
                    COPY(zTransaction)(&z3zE3281, z3zE3282);
                    KILL(zTransaction)(&z3zE3282);
                    goto finish_match_3353;
                  }
                case_3354: ;
                  sail_match_failure("rlp_decode_tx");
                finish_match_3353: ;
                  COPY(zTransaction)(&z3zE3280, z3zE3281);
                  KILL(zTransaction)(&z3zE3281);
                  goto finish_match_3351;
                }
              case_3352: ;
                sail_match_failure("rlp_decode_tx");
              finish_match_3351: ;
                COPY(zTransaction)(&z3zE3279, z3zE3280);
                KILL(zTransaction)(&z3zE3280);
                goto finish_match_3349;
              }
            case_3350: ;
              sail_match_failure("rlp_decode_tx");
            finish_match_3349: ;
              COPY(zTransaction)(&z3zE3278, z3zE3279);
              KILL(zTransaction)(&z3zE3279);
              goto finish_match_3347;
            }
          case_3348: ;
            sail_match_failure("rlp_decode_tx");
          finish_match_3347: ;
            COPY(zTransaction)(&z3zE3277, z3zE3278);
            KILL(zTransaction)(&z3zE3278);
            goto finish_match_3345;
          }
        case_3346: ;
          sail_match_failure("rlp_decode_tx");
        finish_match_3345: ;
          COPY(zTransaction)(&z3zE3276, z3zE3277);
          KILL(zTransaction)(&z3zE3277);
          goto finish_match_3343;
        }
      case_3344: ;
        sail_match_failure("rlp_decode_tx");
      finish_match_3343: ;
        COPY(zTransaction)(&z3zE3275, z3zE3276);
        KILL(zTransaction)(&z3zE3276);
        goto finish_match_3341;
      }
    case_3342: ;
      sail_match_failure("rlp_decode_tx");
    finish_match_3341: ;
      COPY(zTransaction)(&z3zE3274, z3zE3275);
      KILL(zTransaction)(&z3zE3275);
      goto finish_match_3339;
    }
  case_3340: ;
    sail_match_failure("rlp_decode_tx");
  finish_match_3339: ;
    COPY(zTransaction)(&z3zE3205, z3zE3274);
    KILL(zTransaction)(&z3zE3274);
    goto finish_match_3274;
  }
case_3338: ;
  {
    uint64_t z3zE3384;
    z3zE3384 = zttype;
    bool z3zE3273;
    z3zE3273 = (z3zE3384 == UINT64_C(0x03));
    if (!(z3zE3273)) {  goto case_3307;  }
    struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3337;
    {
      z2zE3337 = zrlp_cursor_pop(zfields);
      if (have_exception) {
        KILL(zTransaction)(&z3zE3205);
        goto end_block_exception_3410;
      }
    }
    struct zTransaction z3zE3240;
    CREATE(zTransaction)(&z3zE3240);
    {
      struct zRlpFieldRefFields z3zE3385;
      z3zE3385 = z2zE3337.ztup0;
      struct zRlpCursorFields z1zE41;
      z1zE41 = z2zE3337.ztup1;
      struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3338;
      {
        z2zE3338 = zrlp_cursor_pop(z1zE41);
        if (have_exception) {
          KILL(zTransaction)(&z3zE3205);
          KILL(zTransaction)(&z3zE3240);
          goto end_block_exception_3410;
        }
      }
      struct zTransaction z3zE3241;
      CREATE(zTransaction)(&z3zE3241);
      {
        struct zRlpFieldRefFields z3zE3386;
        z3zE3386 = z2zE3338.ztup0;
        struct zRlpCursorFields z1zE42;
        z1zE42 = z2zE3338.ztup1;
        struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3339;
        {
          z2zE3339 = zrlp_cursor_pop(z1zE42);
          if (have_exception) {
            KILL(zTransaction)(&z3zE3205);
            KILL(zTransaction)(&z3zE3240);
            KILL(zTransaction)(&z3zE3241);
            goto end_block_exception_3410;
          }
        }
        struct zTransaction z3zE3242;
        CREATE(zTransaction)(&z3zE3242);
        {
          struct zRlpFieldRefFields z3zE3387;
          z3zE3387 = z2zE3339.ztup0;
          struct zRlpCursorFields z1zE43;
          z1zE43 = z2zE3339.ztup1;
          struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3340;
          {
            z2zE3340 = zrlp_cursor_pop(z1zE43);
            if (have_exception) {
              KILL(zTransaction)(&z3zE3205);
              KILL(zTransaction)(&z3zE3240);
              KILL(zTransaction)(&z3zE3241);
              KILL(zTransaction)(&z3zE3242);
              goto end_block_exception_3410;
            }
          }
          struct zTransaction z3zE3243;
          CREATE(zTransaction)(&z3zE3243);
          {
            struct zRlpFieldRefFields z3zE3388;
            z3zE3388 = z2zE3340.ztup0;
            struct zRlpCursorFields z1zE44;
            z1zE44 = z2zE3340.ztup1;
            struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3341;
            {
              z2zE3341 = zrlp_cursor_pop(z1zE44);
              if (have_exception) {
                KILL(zTransaction)(&z3zE3205);
                KILL(zTransaction)(&z3zE3240);
                KILL(zTransaction)(&z3zE3241);
                KILL(zTransaction)(&z3zE3242);
                KILL(zTransaction)(&z3zE3243);
                goto end_block_exception_3410;
              }
            }
            struct zTransaction z3zE3244;
            CREATE(zTransaction)(&z3zE3244);
            {
              struct zRlpFieldRefFields z3zE3389;
              z3zE3389 = z2zE3341.ztup0;
              struct zRlpCursorFields z1zE45;
              z1zE45 = z2zE3341.ztup1;
              struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3342;
              {
                z2zE3342 = zrlp_cursor_pop(z1zE45);
                if (have_exception) {
                  KILL(zTransaction)(&z3zE3205);
                  KILL(zTransaction)(&z3zE3240);
                  KILL(zTransaction)(&z3zE3241);
                  KILL(zTransaction)(&z3zE3242);
                  KILL(zTransaction)(&z3zE3243);
                  KILL(zTransaction)(&z3zE3244);
                  goto end_block_exception_3410;
                }
              }
              struct zTransaction z3zE3245;
              CREATE(zTransaction)(&z3zE3245);
              {
                struct zRlpFieldRefFields z3zE3390;
                z3zE3390 = z2zE3342.ztup0;
                struct zRlpCursorFields z1zE46;
                z1zE46 = z2zE3342.ztup1;
                struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3343;
                {
                  z2zE3343 = zrlp_cursor_pop(z1zE46);
                  if (have_exception) {
                    KILL(zTransaction)(&z3zE3205);
                    KILL(zTransaction)(&z3zE3240);
                    KILL(zTransaction)(&z3zE3241);
                    KILL(zTransaction)(&z3zE3242);
                    KILL(zTransaction)(&z3zE3243);
                    KILL(zTransaction)(&z3zE3244);
                    KILL(zTransaction)(&z3zE3245);
                    goto end_block_exception_3410;
                  }
                }
                struct zTransaction z3zE3246;
                CREATE(zTransaction)(&z3zE3246);
                {
                  struct zRlpFieldRefFields z3zE3391;
                  z3zE3391 = z2zE3343.ztup0;
                  struct zRlpCursorFields z1zE47;
                  z1zE47 = z2zE3343.ztup1;
                  struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3344;
                  {
                    z2zE3344 = zrlp_cursor_pop(z1zE47);
                    if (have_exception) {
                      KILL(zTransaction)(&z3zE3205);
                      KILL(zTransaction)(&z3zE3240);
                      KILL(zTransaction)(&z3zE3241);
                      KILL(zTransaction)(&z3zE3242);
                      KILL(zTransaction)(&z3zE3243);
                      KILL(zTransaction)(&z3zE3244);
                      KILL(zTransaction)(&z3zE3245);
                      KILL(zTransaction)(&z3zE3246);
                      goto end_block_exception_3410;
                    }
                  }
                  struct zTransaction z3zE3247;
                  CREATE(zTransaction)(&z3zE3247);
                  {
                    struct zRlpFieldRefFields z3zE3392;
                    z3zE3392 = z2zE3344.ztup0;
                    struct zRlpCursorFields z1zE48;
                    z1zE48 = z2zE3344.ztup1;
                    struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3345;
                    {
                      z2zE3345 = zrlp_cursor_pop(z1zE48);
                      if (have_exception) {
                        KILL(zTransaction)(&z3zE3205);
                        KILL(zTransaction)(&z3zE3240);
                        KILL(zTransaction)(&z3zE3241);
                        KILL(zTransaction)(&z3zE3242);
                        KILL(zTransaction)(&z3zE3243);
                        KILL(zTransaction)(&z3zE3244);
                        KILL(zTransaction)(&z3zE3245);
                        KILL(zTransaction)(&z3zE3246);
                        KILL(zTransaction)(&z3zE3247);
                        goto end_block_exception_3410;
                      }
                    }
                    struct zTransaction z3zE3248;
                    CREATE(zTransaction)(&z3zE3248);
                    {
                      struct zRlpFieldRefFields z3zE3393;
                      z3zE3393 = z2zE3345.ztup0;
                      struct zRlpCursorFields z1zE49;
                      z1zE49 = z2zE3345.ztup1;
                      struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3346;
                      {
                        z2zE3346 = zrlp_cursor_pop(z1zE49);
                        if (have_exception) {
                          KILL(zTransaction)(&z3zE3205);
                          KILL(zTransaction)(&z3zE3240);
                          KILL(zTransaction)(&z3zE3241);
                          KILL(zTransaction)(&z3zE3242);
                          KILL(zTransaction)(&z3zE3243);
                          KILL(zTransaction)(&z3zE3244);
                          KILL(zTransaction)(&z3zE3245);
                          KILL(zTransaction)(&z3zE3246);
                          KILL(zTransaction)(&z3zE3247);
                          KILL(zTransaction)(&z3zE3248);
                          goto end_block_exception_3410;
                        }
                      }
                      struct zTransaction z3zE3249;
                      CREATE(zTransaction)(&z3zE3249);
                      {
                        struct zRlpFieldRefFields zmbf_f;
                        zmbf_f = z2zE3346.ztup0;
                        struct zRlpCursorFields z1zE50;
                        z1zE50 = z2zE3346.ztup1;
                        struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3347;
                        {
                          z2zE3347 = zrlp_cursor_pop(z1zE50);
                          if (have_exception) {
                            KILL(zTransaction)(&z3zE3205);
                            KILL(zTransaction)(&z3zE3240);
                            KILL(zTransaction)(&z3zE3241);
                            KILL(zTransaction)(&z3zE3242);
                            KILL(zTransaction)(&z3zE3243);
                            KILL(zTransaction)(&z3zE3244);
                            KILL(zTransaction)(&z3zE3245);
                            KILL(zTransaction)(&z3zE3246);
                            KILL(zTransaction)(&z3zE3247);
                            KILL(zTransaction)(&z3zE3248);
                            KILL(zTransaction)(&z3zE3249);
                            goto end_block_exception_3410;
                          }
                        }
                        struct zTransaction z3zE3250;
                        CREATE(zTransaction)(&z3zE3250);
                        {
                          struct zRlpFieldRefFields zbh_f;
                          zbh_f = z2zE3347.ztup0;
                          struct zRlpCursorFields z1zE51;
                          z1zE51 = z2zE3347.ztup1;
                          struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3348;
                          {
                            z2zE3348 = zrlp_cursor_pop(z1zE51);
                            if (have_exception) {
                              KILL(zTransaction)(&z3zE3205);
                              KILL(zTransaction)(&z3zE3240);
                              KILL(zTransaction)(&z3zE3241);
                              KILL(zTransaction)(&z3zE3242);
                              KILL(zTransaction)(&z3zE3243);
                              KILL(zTransaction)(&z3zE3244);
                              KILL(zTransaction)(&z3zE3245);
                              KILL(zTransaction)(&z3zE3246);
                              KILL(zTransaction)(&z3zE3247);
                              KILL(zTransaction)(&z3zE3248);
                              KILL(zTransaction)(&z3zE3249);
                              KILL(zTransaction)(&z3zE3250);
                              goto end_block_exception_3410;
                            }
                          }
                          struct zTransaction z3zE3251;
                          CREATE(zTransaction)(&z3zE3251);
                          {
                            struct zRlpFieldRefFields z3zE3394;
                            z3zE3394 = z2zE3348.ztup0;
                            struct zRlpCursorFields z1zE52;
                            z1zE52 = z2zE3348.ztup1;
                            struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3349;
                            {
                              z2zE3349 = zrlp_cursor_pop(z1zE52);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE3205);
                                KILL(zTransaction)(&z3zE3240);
                                KILL(zTransaction)(&z3zE3241);
                                KILL(zTransaction)(&z3zE3242);
                                KILL(zTransaction)(&z3zE3243);
                                KILL(zTransaction)(&z3zE3244);
                                KILL(zTransaction)(&z3zE3245);
                                KILL(zTransaction)(&z3zE3246);
                                KILL(zTransaction)(&z3zE3247);
                                KILL(zTransaction)(&z3zE3248);
                                KILL(zTransaction)(&z3zE3249);
                                KILL(zTransaction)(&z3zE3250);
                                KILL(zTransaction)(&z3zE3251);
                                goto end_block_exception_3410;
                              }
                            }
                            struct zTransaction z3zE3252;
                            CREATE(zTransaction)(&z3zE3252);
                            {
                              struct zRlpFieldRefFields z3zE3395;
                              z3zE3395 = z2zE3349.ztup0;
                              struct zRlpCursorFields z1zE53;
                              z1zE53 = z2zE3349.ztup1;
                              struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3350;
                              {
                                z2zE3350 = zrlp_cursor_pop(z1zE53);
                                if (have_exception) {
                                  KILL(zTransaction)(&z3zE3205);
                                  KILL(zTransaction)(&z3zE3240);
                                  KILL(zTransaction)(&z3zE3241);
                                  KILL(zTransaction)(&z3zE3242);
                                  KILL(zTransaction)(&z3zE3243);
                                  KILL(zTransaction)(&z3zE3244);
                                  KILL(zTransaction)(&z3zE3245);
                                  KILL(zTransaction)(&z3zE3246);
                                  KILL(zTransaction)(&z3zE3247);
                                  KILL(zTransaction)(&z3zE3248);
                                  KILL(zTransaction)(&z3zE3249);
                                  KILL(zTransaction)(&z3zE3250);
                                  KILL(zTransaction)(&z3zE3251);
                                  KILL(zTransaction)(&z3zE3252);
                                  goto end_block_exception_3410;
                                }
                              }
                              struct zTransaction z3zE3253;
                              CREATE(zTransaction)(&z3zE3253);
                              {
                                struct zRlpFieldRefFields z3zE3396;
                                z3zE3396 = z2zE3350.ztup0;
                                struct zRlpCursorFields z1zE54;
                                z1zE54 = z2zE3350.ztup1;
                                unit z3zE3254;
                                {
                                  z3zE3254 = zrlp_cursor_expect_end(z1zE54);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE3205);
                                    KILL(zTransaction)(&z3zE3240);
                                    KILL(zTransaction)(&z3zE3241);
                                    KILL(zTransaction)(&z3zE3242);
                                    KILL(zTransaction)(&z3zE3243);
                                    KILL(zTransaction)(&z3zE3244);
                                    KILL(zTransaction)(&z3zE3245);
                                    KILL(zTransaction)(&z3zE3246);
                                    KILL(zTransaction)(&z3zE3247);
                                    KILL(zTransaction)(&z3zE3248);
                                    KILL(zTransaction)(&z3zE3249);
                                    KILL(zTransaction)(&z3zE3250);
                                    KILL(zTransaction)(&z3zE3251);
                                    KILL(zTransaction)(&z3zE3252);
                                    KILL(zTransaction)(&z3zE3253);
                                    goto end_block_exception_3410;
                                  }
                                }
                                sail_u256 z3zE3397;
                                {
                                  z3zE3397 = zrlp_ref_word(z3zE3394);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE3205);
                                    KILL(zTransaction)(&z3zE3240);
                                    KILL(zTransaction)(&z3zE3241);
                                    KILL(zTransaction)(&z3zE3242);
                                    KILL(zTransaction)(&z3zE3243);
                                    KILL(zTransaction)(&z3zE3244);
                                    KILL(zTransaction)(&z3zE3245);
                                    KILL(zTransaction)(&z3zE3246);
                                    KILL(zTransaction)(&z3zE3247);
                                    KILL(zTransaction)(&z3zE3248);
                                    KILL(zTransaction)(&z3zE3249);
                                    KILL(zTransaction)(&z3zE3250);
                                    KILL(zTransaction)(&z3zE3251);
                                    KILL(zTransaction)(&z3zE3252);
                                    KILL(zTransaction)(&z3zE3253);
                                    goto end_block_exception_3410;
                                  }
                                }
                                struct ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9 z2zE3351;
                                CREATE(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3351);
                                {
                                  zdecode_access_list(&z2zE3351, z3zE3393);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE3205);
                                    KILL(zTransaction)(&z3zE3240);
                                    KILL(zTransaction)(&z3zE3241);
                                    KILL(zTransaction)(&z3zE3242);
                                    KILL(zTransaction)(&z3zE3243);
                                    KILL(zTransaction)(&z3zE3244);
                                    KILL(zTransaction)(&z3zE3245);
                                    KILL(zTransaction)(&z3zE3246);
                                    KILL(zTransaction)(&z3zE3247);
                                    KILL(zTransaction)(&z3zE3248);
                                    KILL(zTransaction)(&z3zE3249);
                                    KILL(zTransaction)(&z3zE3250);
                                    KILL(zTransaction)(&z3zE3251);
                                    KILL(zTransaction)(&z3zE3252);
                                    KILL(zTransaction)(&z3zE3253);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3351);
                                    goto end_block_exception_3410;
                                  }
                                }
                                struct zTransaction z3zE3255;
                                CREATE(zTransaction)(&z3zE3255);
                                {
                                  zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z3zE3398;
                                  CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3398);
                                  COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3398, z2zE3351.ztup0);
                                  zz5listz8z5structz0zzStorageKeyz9 z3zE3399;
                                  CREATE(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3399);
                                  COPY(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3399, z2zE3351.ztup1);
                                  uint64_t z3zE3400;
                                  z3zE3400 = z2zE3351.ztup2;
                                  uint64_t z3zE3401;
                                  z3zE3401 = z2zE3351.ztup3;
                                  struct zBlobHashes zblob_hashes;
                                  {
                                    zblob_hashes = zdecode_blob_hashes(zbh_f);
                                    if (have_exception) {
                                      KILL(zTransaction)(&z3zE3205);
                                      KILL(zTransaction)(&z3zE3240);
                                      KILL(zTransaction)(&z3zE3241);
                                      KILL(zTransaction)(&z3zE3242);
                                      KILL(zTransaction)(&z3zE3243);
                                      KILL(zTransaction)(&z3zE3244);
                                      KILL(zTransaction)(&z3zE3245);
                                      KILL(zTransaction)(&z3zE3246);
                                      KILL(zTransaction)(&z3zE3247);
                                      KILL(zTransaction)(&z3zE3248);
                                      KILL(zTransaction)(&z3zE3249);
                                      KILL(zTransaction)(&z3zE3250);
                                      KILL(zTransaction)(&z3zE3251);
                                      KILL(zTransaction)(&z3zE3252);
                                      KILL(zTransaction)(&z3zE3253);
                                      KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3351);
                                      KILL(zTransaction)(&z3zE3255);
                                      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3398);
                                      KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3399);
                                      goto end_block_exception_3410;
                                    }
                                  }
                                  sail_u256 z2zE3352;
                                  {
                                    z2zE3352 = zrlp_ref_uint_word(z3zE3386);
                                    if (have_exception) {
                                      KILL(zTransaction)(&z3zE3205);
                                      KILL(zTransaction)(&z3zE3240);
                                      KILL(zTransaction)(&z3zE3241);
                                      KILL(zTransaction)(&z3zE3242);
                                      KILL(zTransaction)(&z3zE3243);
                                      KILL(zTransaction)(&z3zE3244);
                                      KILL(zTransaction)(&z3zE3245);
                                      KILL(zTransaction)(&z3zE3246);
                                      KILL(zTransaction)(&z3zE3247);
                                      KILL(zTransaction)(&z3zE3248);
                                      KILL(zTransaction)(&z3zE3249);
                                      KILL(zTransaction)(&z3zE3250);
                                      KILL(zTransaction)(&z3zE3251);
                                      KILL(zTransaction)(&z3zE3252);
                                      KILL(zTransaction)(&z3zE3253);
                                      KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3351);
                                      KILL(zTransaction)(&z3zE3255);
                                      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3398);
                                      KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3399);
                                      goto end_block_exception_3410;
                                    }
                                  }
                                  uint64_t z2zE3353;
                                  {
                                    z2zE3353 = zrlp_ref_chain_identifier(z3zE3385);
                                    if (have_exception) {
                                      KILL(zTransaction)(&z3zE3205);
                                      KILL(zTransaction)(&z3zE3240);
                                      KILL(zTransaction)(&z3zE3241);
                                      KILL(zTransaction)(&z3zE3242);
                                      KILL(zTransaction)(&z3zE3243);
                                      KILL(zTransaction)(&z3zE3244);
                                      KILL(zTransaction)(&z3zE3245);
                                      KILL(zTransaction)(&z3zE3246);
                                      KILL(zTransaction)(&z3zE3247);
                                      KILL(zTransaction)(&z3zE3248);
                                      KILL(zTransaction)(&z3zE3249);
                                      KILL(zTransaction)(&z3zE3250);
                                      KILL(zTransaction)(&z3zE3251);
                                      KILL(zTransaction)(&z3zE3252);
                                      KILL(zTransaction)(&z3zE3253);
                                      KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3351);
                                      KILL(zTransaction)(&z3zE3255);
                                      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3398);
                                      KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3399);
                                      goto end_block_exception_3410;
                                    }
                                  }
                                  uint64_t z2zE3354;
                                  {
                                    z2zE3354 = zrlp_ref_gas(z3zE3389, zfork);
                                    if (have_exception) {
                                      KILL(zTransaction)(&z3zE3205);
                                      KILL(zTransaction)(&z3zE3240);
                                      KILL(zTransaction)(&z3zE3241);
                                      KILL(zTransaction)(&z3zE3242);
                                      KILL(zTransaction)(&z3zE3243);
                                      KILL(zTransaction)(&z3zE3244);
                                      KILL(zTransaction)(&z3zE3245);
                                      KILL(zTransaction)(&z3zE3246);
                                      KILL(zTransaction)(&z3zE3247);
                                      KILL(zTransaction)(&z3zE3248);
                                      KILL(zTransaction)(&z3zE3249);
                                      KILL(zTransaction)(&z3zE3250);
                                      KILL(zTransaction)(&z3zE3251);
                                      KILL(zTransaction)(&z3zE3252);
                                      KILL(zTransaction)(&z3zE3253);
                                      KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3351);
                                      KILL(zTransaction)(&z3zE3255);
                                      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3398);
                                      KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3399);
                                      goto end_block_exception_3410;
                                    }
                                  }
                                  bool z2zE3356;
                                  {
                                    uint64_t z2zE3355;
                                    z2zE3355 = z3zE3390.zcontent_len;
                                    z2zE3356 = (z2zE3355 == UINT64_C(0));
                                  }
                                  sail_fixed_bytes_20 z2zE3358;
                                  {
                                    sail_u256 z2zE3357;
                                    {
                                      z2zE3357 = zrlp_ref_word(z3zE3390);
                                      if (have_exception) {
                                        KILL(zTransaction)(&z3zE3205);
                                        KILL(zTransaction)(&z3zE3240);
                                        KILL(zTransaction)(&z3zE3241);
                                        KILL(zTransaction)(&z3zE3242);
                                        KILL(zTransaction)(&z3zE3243);
                                        KILL(zTransaction)(&z3zE3244);
                                        KILL(zTransaction)(&z3zE3245);
                                        KILL(zTransaction)(&z3zE3246);
                                        KILL(zTransaction)(&z3zE3247);
                                        KILL(zTransaction)(&z3zE3248);
                                        KILL(zTransaction)(&z3zE3249);
                                        KILL(zTransaction)(&z3zE3250);
                                        KILL(zTransaction)(&z3zE3251);
                                        KILL(zTransaction)(&z3zE3252);
                                        KILL(zTransaction)(&z3zE3253);
                                        KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3351);
                                        KILL(zTransaction)(&z3zE3255);
                                        KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3398);
                                        KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3399);
                                        goto end_block_exception_3410;
                                      }
                                    }
                                    z2zE3358 = zword_to_address(z2zE3357);
                                  }
                                  sail_u256 z2zE3359;
                                  {
                                    z2zE3359 = zrlp_ref_uint_word(z3zE3391);
                                    if (have_exception) {
                                      KILL(zTransaction)(&z3zE3205);
                                      KILL(zTransaction)(&z3zE3240);
                                      KILL(zTransaction)(&z3zE3241);
                                      KILL(zTransaction)(&z3zE3242);
                                      KILL(zTransaction)(&z3zE3243);
                                      KILL(zTransaction)(&z3zE3244);
                                      KILL(zTransaction)(&z3zE3245);
                                      KILL(zTransaction)(&z3zE3246);
                                      KILL(zTransaction)(&z3zE3247);
                                      KILL(zTransaction)(&z3zE3248);
                                      KILL(zTransaction)(&z3zE3249);
                                      KILL(zTransaction)(&z3zE3250);
                                      KILL(zTransaction)(&z3zE3251);
                                      KILL(zTransaction)(&z3zE3252);
                                      KILL(zTransaction)(&z3zE3253);
                                      KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3351);
                                      KILL(zTransaction)(&z3zE3255);
                                      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3398);
                                      KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3399);
                                      goto end_block_exception_3410;
                                    }
                                  }
                                  struct zByteSliceFields z2zE3360;
                                  {
                                    z2zE3360 = ztx_input_span(z3zE3392);
                                    if (have_exception) {
                                      KILL(zTransaction)(&z3zE3205);
                                      KILL(zTransaction)(&z3zE3240);
                                      KILL(zTransaction)(&z3zE3241);
                                      KILL(zTransaction)(&z3zE3242);
                                      KILL(zTransaction)(&z3zE3243);
                                      KILL(zTransaction)(&z3zE3244);
                                      KILL(zTransaction)(&z3zE3245);
                                      KILL(zTransaction)(&z3zE3246);
                                      KILL(zTransaction)(&z3zE3247);
                                      KILL(zTransaction)(&z3zE3248);
                                      KILL(zTransaction)(&z3zE3249);
                                      KILL(zTransaction)(&z3zE3250);
                                      KILL(zTransaction)(&z3zE3251);
                                      KILL(zTransaction)(&z3zE3252);
                                      KILL(zTransaction)(&z3zE3253);
                                      KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3351);
                                      KILL(zTransaction)(&z3zE3255);
                                      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3398);
                                      KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3399);
                                      goto end_block_exception_3410;
                                    }
                                  }
                                  sail_u256 z2zE3361;
                                  {
                                    z2zE3361 = zrlp_ref_uint_word(z3zE3388);
                                    if (have_exception) {
                                      KILL(zTransaction)(&z3zE3205);
                                      KILL(zTransaction)(&z3zE3240);
                                      KILL(zTransaction)(&z3zE3241);
                                      KILL(zTransaction)(&z3zE3242);
                                      KILL(zTransaction)(&z3zE3243);
                                      KILL(zTransaction)(&z3zE3244);
                                      KILL(zTransaction)(&z3zE3245);
                                      KILL(zTransaction)(&z3zE3246);
                                      KILL(zTransaction)(&z3zE3247);
                                      KILL(zTransaction)(&z3zE3248);
                                      KILL(zTransaction)(&z3zE3249);
                                      KILL(zTransaction)(&z3zE3250);
                                      KILL(zTransaction)(&z3zE3251);
                                      KILL(zTransaction)(&z3zE3252);
                                      KILL(zTransaction)(&z3zE3253);
                                      KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3351);
                                      KILL(zTransaction)(&z3zE3255);
                                      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3398);
                                      KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3399);
                                      goto end_block_exception_3410;
                                    }
                                  }
                                  sail_u256 z2zE3362;
                                  {
                                    z2zE3362 = zrlp_ref_uint_word(zmbf_f);
                                    if (have_exception) {
                                      KILL(zTransaction)(&z3zE3205);
                                      KILL(zTransaction)(&z3zE3240);
                                      KILL(zTransaction)(&z3zE3241);
                                      KILL(zTransaction)(&z3zE3242);
                                      KILL(zTransaction)(&z3zE3243);
                                      KILL(zTransaction)(&z3zE3244);
                                      KILL(zTransaction)(&z3zE3245);
                                      KILL(zTransaction)(&z3zE3246);
                                      KILL(zTransaction)(&z3zE3247);
                                      KILL(zTransaction)(&z3zE3248);
                                      KILL(zTransaction)(&z3zE3249);
                                      KILL(zTransaction)(&z3zE3250);
                                      KILL(zTransaction)(&z3zE3251);
                                      KILL(zTransaction)(&z3zE3252);
                                      KILL(zTransaction)(&z3zE3253);
                                      KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3351);
                                      KILL(zTransaction)(&z3zE3255);
                                      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3398);
                                      KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3399);
                                      goto end_block_exception_3410;
                                    }
                                  }
                                  sail_u256 z2zE3363;
                                  {
                                    z2zE3363 = zrlp_ref_uint_word(z3zE3387);
                                    if (have_exception) {
                                      KILL(zTransaction)(&z3zE3205);
                                      KILL(zTransaction)(&z3zE3240);
                                      KILL(zTransaction)(&z3zE3241);
                                      KILL(zTransaction)(&z3zE3242);
                                      KILL(zTransaction)(&z3zE3243);
                                      KILL(zTransaction)(&z3zE3244);
                                      KILL(zTransaction)(&z3zE3245);
                                      KILL(zTransaction)(&z3zE3246);
                                      KILL(zTransaction)(&z3zE3247);
                                      KILL(zTransaction)(&z3zE3248);
                                      KILL(zTransaction)(&z3zE3249);
                                      KILL(zTransaction)(&z3zE3250);
                                      KILL(zTransaction)(&z3zE3251);
                                      KILL(zTransaction)(&z3zE3252);
                                      KILL(zTransaction)(&z3zE3253);
                                      KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3351);
                                      KILL(zTransaction)(&z3zE3255);
                                      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3398);
                                      KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3399);
                                      goto end_block_exception_3410;
                                    }
                                  }
                                  sail_fixed_bytes_32 z2zE3365;
                                  {
                                    struct zByteSliceFields z2zE3364;
                                    {
                                      z2zE3364 = ztx_sig_span(z3zE3385, z3zE3394);
                                      if (have_exception) {
                                        KILL(zTransaction)(&z3zE3205);
                                        KILL(zTransaction)(&z3zE3240);
                                        KILL(zTransaction)(&z3zE3241);
                                        KILL(zTransaction)(&z3zE3242);
                                        KILL(zTransaction)(&z3zE3243);
                                        KILL(zTransaction)(&z3zE3244);
                                        KILL(zTransaction)(&z3zE3245);
                                        KILL(zTransaction)(&z3zE3246);
                                        KILL(zTransaction)(&z3zE3247);
                                        KILL(zTransaction)(&z3zE3248);
                                        KILL(zTransaction)(&z3zE3249);
                                        KILL(zTransaction)(&z3zE3250);
                                        KILL(zTransaction)(&z3zE3251);
                                        KILL(zTransaction)(&z3zE3252);
                                        KILL(zTransaction)(&z3zE3253);
                                        KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3351);
                                        KILL(zTransaction)(&z3zE3255);
                                        KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3398);
                                        KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3399);
                                        goto end_block_exception_3410;
                                      }
                                    }
                                    {
                                      z2zE3365 = ztx_signing_hash(zBlobTx, z2zE3364, z3zE3397);
                                      if (have_exception) {
                                        KILL(zTransaction)(&z3zE3205);
                                        KILL(zTransaction)(&z3zE3240);
                                        KILL(zTransaction)(&z3zE3241);
                                        KILL(zTransaction)(&z3zE3242);
                                        KILL(zTransaction)(&z3zE3243);
                                        KILL(zTransaction)(&z3zE3244);
                                        KILL(zTransaction)(&z3zE3245);
                                        KILL(zTransaction)(&z3zE3246);
                                        KILL(zTransaction)(&z3zE3247);
                                        KILL(zTransaction)(&z3zE3248);
                                        KILL(zTransaction)(&z3zE3249);
                                        KILL(zTransaction)(&z3zE3250);
                                        KILL(zTransaction)(&z3zE3251);
                                        KILL(zTransaction)(&z3zE3252);
                                        KILL(zTransaction)(&z3zE3253);
                                        KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3351);
                                        KILL(zTransaction)(&z3zE3255);
                                        KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3398);
                                        KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3399);
                                        goto end_block_exception_3410;
                                      }
                                    }
                                  }
                                  sail_u256 z2zE3366;
                                  {
                                    z2zE3366 = zrlp_ref_uint_word(z3zE3395);
                                    if (have_exception) {
                                      KILL(zTransaction)(&z3zE3205);
                                      KILL(zTransaction)(&z3zE3240);
                                      KILL(zTransaction)(&z3zE3241);
                                      KILL(zTransaction)(&z3zE3242);
                                      KILL(zTransaction)(&z3zE3243);
                                      KILL(zTransaction)(&z3zE3244);
                                      KILL(zTransaction)(&z3zE3245);
                                      KILL(zTransaction)(&z3zE3246);
                                      KILL(zTransaction)(&z3zE3247);
                                      KILL(zTransaction)(&z3zE3248);
                                      KILL(zTransaction)(&z3zE3249);
                                      KILL(zTransaction)(&z3zE3250);
                                      KILL(zTransaction)(&z3zE3251);
                                      KILL(zTransaction)(&z3zE3252);
                                      KILL(zTransaction)(&z3zE3253);
                                      KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3351);
                                      KILL(zTransaction)(&z3zE3255);
                                      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3398);
                                      KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3399);
                                      goto end_block_exception_3410;
                                    }
                                  }
                                  sail_u256 z2zE3367;
                                  {
                                    z2zE3367 = zrlp_ref_uint_word(z3zE3396);
                                    if (have_exception) {
                                      KILL(zTransaction)(&z3zE3205);
                                      KILL(zTransaction)(&z3zE3240);
                                      KILL(zTransaction)(&z3zE3241);
                                      KILL(zTransaction)(&z3zE3242);
                                      KILL(zTransaction)(&z3zE3243);
                                      KILL(zTransaction)(&z3zE3244);
                                      KILL(zTransaction)(&z3zE3245);
                                      KILL(zTransaction)(&z3zE3246);
                                      KILL(zTransaction)(&z3zE3247);
                                      KILL(zTransaction)(&z3zE3248);
                                      KILL(zTransaction)(&z3zE3249);
                                      KILL(zTransaction)(&z3zE3250);
                                      KILL(zTransaction)(&z3zE3251);
                                      KILL(zTransaction)(&z3zE3252);
                                      KILL(zTransaction)(&z3zE3253);
                                      KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3351);
                                      KILL(zTransaction)(&z3zE3255);
                                      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3398);
                                      KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3399);
                                      goto end_block_exception_3410;
                                    }
                                  }
                                  z3zE3255.zaccess_list_address_count = z3zE3400;
                                  COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE3255)->zaccess_list_addresses), z3zE3398);
                                  z3zE3255.zaccess_list_slot_count = z3zE3401;
                                  COPY(zz5listz8z5structz0zzStorageKeyz9)(&((&z3zE3255)->zaccess_list_slots), z3zE3399);
                                  z3zE3255.zauthorizzation_count = UINT64_C(0);
                                  zz5listz8z5structz0zzAuthorizzzzationz9 z3zE3257;
                                  CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&z3zE3257);
                                  COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&((&z3zE3255)->zauthorizzations), z3zE3257);
                                  KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z3zE3257);
                                  z3zE3255.zblob_hashes = zblob_hashes;
                                  z3zE3255.zchain_id = z2zE3353;
                                  z3zE3255.zgas_limit = z2zE3354;
                                  z3zE3255.zinput_src = z2zE3360;
                                  z3zE3255.zis_create = z2zE3356;
                                  z3zE3255.zmax_blob_fee = z2zE3362;
                                  z3zE3255.zmax_fee = z2zE3361;
                                  z3zE3255.zmax_priority_fee = z2zE3363;
                                  z3zE3255.znonce = z2zE3352;
                                  z3zE3255.zpubkey = zpubkey;
                                  z3zE3255.zraw = ztx;
                                  z3zE3255.zrecipient = z2zE3358;
                                  z3zE3255.zsender = zsender;
                                  z3zE3255.zsig_r = z2zE3366;
                                  z3zE3255.zsig_s = z2zE3367;
                                  z3zE3255.zsig_v = z3zE3397;
                                  z3zE3255.zsigning_hash = z2zE3365;
                                  z3zE3255.ztx_type = zBlobTx;
                                  z3zE3255.zvalue = z2zE3359;
                                  KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3399);
                                  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3398);
                                  goto finish_match_3336;
                                }
                              case_3337: ;
                                sail_match_failure("rlp_decode_tx");
                              finish_match_3336: ;
                                COPY(zTransaction)(&z3zE3253, z3zE3255);
                                KILL(zTransaction)(&z3zE3255);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3351);
                                goto finish_match_3334;
                              }
                            case_3335: ;
                              sail_match_failure("rlp_decode_tx");
                            finish_match_3334: ;
                              COPY(zTransaction)(&z3zE3252, z3zE3253);
                              KILL(zTransaction)(&z3zE3253);
                              goto finish_match_3332;
                            }
                          case_3333: ;
                            sail_match_failure("rlp_decode_tx");
                          finish_match_3332: ;
                            COPY(zTransaction)(&z3zE3251, z3zE3252);
                            KILL(zTransaction)(&z3zE3252);
                            goto finish_match_3330;
                          }
                        case_3331: ;
                          sail_match_failure("rlp_decode_tx");
                        finish_match_3330: ;
                          COPY(zTransaction)(&z3zE3250, z3zE3251);
                          KILL(zTransaction)(&z3zE3251);
                          goto finish_match_3328;
                        }
                      case_3329: ;
                        sail_match_failure("rlp_decode_tx");
                      finish_match_3328: ;
                        COPY(zTransaction)(&z3zE3249, z3zE3250);
                        KILL(zTransaction)(&z3zE3250);
                        goto finish_match_3326;
                      }
                    case_3327: ;
                      sail_match_failure("rlp_decode_tx");
                    finish_match_3326: ;
                      COPY(zTransaction)(&z3zE3248, z3zE3249);
                      KILL(zTransaction)(&z3zE3249);
                      goto finish_match_3324;
                    }
                  case_3325: ;
                    sail_match_failure("rlp_decode_tx");
                  finish_match_3324: ;
                    COPY(zTransaction)(&z3zE3247, z3zE3248);
                    KILL(zTransaction)(&z3zE3248);
                    goto finish_match_3322;
                  }
                case_3323: ;
                  sail_match_failure("rlp_decode_tx");
                finish_match_3322: ;
                  COPY(zTransaction)(&z3zE3246, z3zE3247);
                  KILL(zTransaction)(&z3zE3247);
                  goto finish_match_3320;
                }
              case_3321: ;
                sail_match_failure("rlp_decode_tx");
              finish_match_3320: ;
                COPY(zTransaction)(&z3zE3245, z3zE3246);
                KILL(zTransaction)(&z3zE3246);
                goto finish_match_3318;
              }
            case_3319: ;
              sail_match_failure("rlp_decode_tx");
            finish_match_3318: ;
              COPY(zTransaction)(&z3zE3244, z3zE3245);
              KILL(zTransaction)(&z3zE3245);
              goto finish_match_3316;
            }
          case_3317: ;
            sail_match_failure("rlp_decode_tx");
          finish_match_3316: ;
            COPY(zTransaction)(&z3zE3243, z3zE3244);
            KILL(zTransaction)(&z3zE3244);
            goto finish_match_3314;
          }
        case_3315: ;
          sail_match_failure("rlp_decode_tx");
        finish_match_3314: ;
          COPY(zTransaction)(&z3zE3242, z3zE3243);
          KILL(zTransaction)(&z3zE3243);
          goto finish_match_3312;
        }
      case_3313: ;
        sail_match_failure("rlp_decode_tx");
      finish_match_3312: ;
        COPY(zTransaction)(&z3zE3241, z3zE3242);
        KILL(zTransaction)(&z3zE3242);
        goto finish_match_3310;
      }
    case_3311: ;
      sail_match_failure("rlp_decode_tx");
    finish_match_3310: ;
      COPY(zTransaction)(&z3zE3240, z3zE3241);
      KILL(zTransaction)(&z3zE3241);
      goto finish_match_3308;
    }
  case_3309: ;
    sail_match_failure("rlp_decode_tx");
  finish_match_3308: ;
    COPY(zTransaction)(&z3zE3205, z3zE3240);
    KILL(zTransaction)(&z3zE3240);
    goto finish_match_3274;
  }
case_3307: ;
  {
    uint64_t z3zE3402;
    z3zE3402 = zttype;
    bool z3zE3239;
    z3zE3239 = (z3zE3402 == UINT64_C(0x04));
    if (!(z3zE3239)) {  goto case_3276;  }
    struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3368;
    {
      z2zE3368 = zrlp_cursor_pop(zfields);
      if (have_exception) {
        KILL(zTransaction)(&z3zE3205);
        goto end_block_exception_3410;
      }
    }
    struct zTransaction z3zE3207;
    CREATE(zTransaction)(&z3zE3207);
    {
      struct zRlpFieldRefFields z3zE3403;
      z3zE3403 = z2zE3368.ztup0;
      struct zRlpCursorFields z1zE55;
      z1zE55 = z2zE3368.ztup1;
      struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3369;
      {
        z2zE3369 = zrlp_cursor_pop(z1zE55);
        if (have_exception) {
          KILL(zTransaction)(&z3zE3205);
          KILL(zTransaction)(&z3zE3207);
          goto end_block_exception_3410;
        }
      }
      struct zTransaction z3zE3208;
      CREATE(zTransaction)(&z3zE3208);
      {
        struct zRlpFieldRefFields z3zE3404;
        z3zE3404 = z2zE3369.ztup0;
        struct zRlpCursorFields z1zE56;
        z1zE56 = z2zE3369.ztup1;
        struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3370;
        {
          z2zE3370 = zrlp_cursor_pop(z1zE56);
          if (have_exception) {
            KILL(zTransaction)(&z3zE3205);
            KILL(zTransaction)(&z3zE3207);
            KILL(zTransaction)(&z3zE3208);
            goto end_block_exception_3410;
          }
        }
        struct zTransaction z3zE3209;
        CREATE(zTransaction)(&z3zE3209);
        {
          struct zRlpFieldRefFields z3zE3405;
          z3zE3405 = z2zE3370.ztup0;
          struct zRlpCursorFields z1zE57;
          z1zE57 = z2zE3370.ztup1;
          struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3371;
          {
            z2zE3371 = zrlp_cursor_pop(z1zE57);
            if (have_exception) {
              KILL(zTransaction)(&z3zE3205);
              KILL(zTransaction)(&z3zE3207);
              KILL(zTransaction)(&z3zE3208);
              KILL(zTransaction)(&z3zE3209);
              goto end_block_exception_3410;
            }
          }
          struct zTransaction z3zE3210;
          CREATE(zTransaction)(&z3zE3210);
          {
            struct zRlpFieldRefFields z3zE3406;
            z3zE3406 = z2zE3371.ztup0;
            struct zRlpCursorFields z1zE58;
            z1zE58 = z2zE3371.ztup1;
            struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3372;
            {
              z2zE3372 = zrlp_cursor_pop(z1zE58);
              if (have_exception) {
                KILL(zTransaction)(&z3zE3205);
                KILL(zTransaction)(&z3zE3207);
                KILL(zTransaction)(&z3zE3208);
                KILL(zTransaction)(&z3zE3209);
                KILL(zTransaction)(&z3zE3210);
                goto end_block_exception_3410;
              }
            }
            struct zTransaction z3zE3211;
            CREATE(zTransaction)(&z3zE3211);
            {
              struct zRlpFieldRefFields z3zE3407;
              z3zE3407 = z2zE3372.ztup0;
              struct zRlpCursorFields z1zE59;
              z1zE59 = z2zE3372.ztup1;
              struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3373;
              {
                z2zE3373 = zrlp_cursor_pop(z1zE59);
                if (have_exception) {
                  KILL(zTransaction)(&z3zE3205);
                  KILL(zTransaction)(&z3zE3207);
                  KILL(zTransaction)(&z3zE3208);
                  KILL(zTransaction)(&z3zE3209);
                  KILL(zTransaction)(&z3zE3210);
                  KILL(zTransaction)(&z3zE3211);
                  goto end_block_exception_3410;
                }
              }
              struct zTransaction z3zE3212;
              CREATE(zTransaction)(&z3zE3212);
              {
                struct zRlpFieldRefFields z3zE3408;
                z3zE3408 = z2zE3373.ztup0;
                struct zRlpCursorFields z1zE60;
                z1zE60 = z2zE3373.ztup1;
                struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3374;
                {
                  z2zE3374 = zrlp_cursor_pop(z1zE60);
                  if (have_exception) {
                    KILL(zTransaction)(&z3zE3205);
                    KILL(zTransaction)(&z3zE3207);
                    KILL(zTransaction)(&z3zE3208);
                    KILL(zTransaction)(&z3zE3209);
                    KILL(zTransaction)(&z3zE3210);
                    KILL(zTransaction)(&z3zE3211);
                    KILL(zTransaction)(&z3zE3212);
                    goto end_block_exception_3410;
                  }
                }
                struct zTransaction z3zE3213;
                CREATE(zTransaction)(&z3zE3213);
                {
                  struct zRlpFieldRefFields z3zE3409;
                  z3zE3409 = z2zE3374.ztup0;
                  struct zRlpCursorFields z1zE61;
                  z1zE61 = z2zE3374.ztup1;
                  struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3375;
                  {
                    z2zE3375 = zrlp_cursor_pop(z1zE61);
                    if (have_exception) {
                      KILL(zTransaction)(&z3zE3205);
                      KILL(zTransaction)(&z3zE3207);
                      KILL(zTransaction)(&z3zE3208);
                      KILL(zTransaction)(&z3zE3209);
                      KILL(zTransaction)(&z3zE3210);
                      KILL(zTransaction)(&z3zE3211);
                      KILL(zTransaction)(&z3zE3212);
                      KILL(zTransaction)(&z3zE3213);
                      goto end_block_exception_3410;
                    }
                  }
                  struct zTransaction z3zE3214;
                  CREATE(zTransaction)(&z3zE3214);
                  {
                    struct zRlpFieldRefFields z3zE3410;
                    z3zE3410 = z2zE3375.ztup0;
                    struct zRlpCursorFields z1zE62;
                    z1zE62 = z2zE3375.ztup1;
                    struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3376;
                    {
                      z2zE3376 = zrlp_cursor_pop(z1zE62);
                      if (have_exception) {
                        KILL(zTransaction)(&z3zE3205);
                        KILL(zTransaction)(&z3zE3207);
                        KILL(zTransaction)(&z3zE3208);
                        KILL(zTransaction)(&z3zE3209);
                        KILL(zTransaction)(&z3zE3210);
                        KILL(zTransaction)(&z3zE3211);
                        KILL(zTransaction)(&z3zE3212);
                        KILL(zTransaction)(&z3zE3213);
                        KILL(zTransaction)(&z3zE3214);
                        goto end_block_exception_3410;
                      }
                    }
                    struct zTransaction z3zE3215;
                    CREATE(zTransaction)(&z3zE3215);
                    {
                      struct zRlpFieldRefFields z3zE3411;
                      z3zE3411 = z2zE3376.ztup0;
                      struct zRlpCursorFields z1zE63;
                      z1zE63 = z2zE3376.ztup1;
                      struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3377;
                      {
                        z2zE3377 = zrlp_cursor_pop(z1zE63);
                        if (have_exception) {
                          KILL(zTransaction)(&z3zE3205);
                          KILL(zTransaction)(&z3zE3207);
                          KILL(zTransaction)(&z3zE3208);
                          KILL(zTransaction)(&z3zE3209);
                          KILL(zTransaction)(&z3zE3210);
                          KILL(zTransaction)(&z3zE3211);
                          KILL(zTransaction)(&z3zE3212);
                          KILL(zTransaction)(&z3zE3213);
                          KILL(zTransaction)(&z3zE3214);
                          KILL(zTransaction)(&z3zE3215);
                          goto end_block_exception_3410;
                        }
                      }
                      struct zTransaction z3zE3216;
                      CREATE(zTransaction)(&z3zE3216);
                      {
                        struct zRlpFieldRefFields zauth_f;
                        zauth_f = z2zE3377.ztup0;
                        struct zRlpCursorFields z1zE64;
                        z1zE64 = z2zE3377.ztup1;
                        struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3378;
                        {
                          z2zE3378 = zrlp_cursor_pop(z1zE64);
                          if (have_exception) {
                            KILL(zTransaction)(&z3zE3205);
                            KILL(zTransaction)(&z3zE3207);
                            KILL(zTransaction)(&z3zE3208);
                            KILL(zTransaction)(&z3zE3209);
                            KILL(zTransaction)(&z3zE3210);
                            KILL(zTransaction)(&z3zE3211);
                            KILL(zTransaction)(&z3zE3212);
                            KILL(zTransaction)(&z3zE3213);
                            KILL(zTransaction)(&z3zE3214);
                            KILL(zTransaction)(&z3zE3215);
                            KILL(zTransaction)(&z3zE3216);
                            goto end_block_exception_3410;
                          }
                        }
                        struct zTransaction z3zE3217;
                        CREATE(zTransaction)(&z3zE3217);
                        {
                          struct zRlpFieldRefFields z3zE3412;
                          z3zE3412 = z2zE3378.ztup0;
                          struct zRlpCursorFields z1zE65;
                          z1zE65 = z2zE3378.ztup1;
                          struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3379;
                          {
                            z2zE3379 = zrlp_cursor_pop(z1zE65);
                            if (have_exception) {
                              KILL(zTransaction)(&z3zE3205);
                              KILL(zTransaction)(&z3zE3207);
                              KILL(zTransaction)(&z3zE3208);
                              KILL(zTransaction)(&z3zE3209);
                              KILL(zTransaction)(&z3zE3210);
                              KILL(zTransaction)(&z3zE3211);
                              KILL(zTransaction)(&z3zE3212);
                              KILL(zTransaction)(&z3zE3213);
                              KILL(zTransaction)(&z3zE3214);
                              KILL(zTransaction)(&z3zE3215);
                              KILL(zTransaction)(&z3zE3216);
                              KILL(zTransaction)(&z3zE3217);
                              goto end_block_exception_3410;
                            }
                          }
                          struct zTransaction z3zE3218;
                          CREATE(zTransaction)(&z3zE3218);
                          {
                            struct zRlpFieldRefFields z3zE3413;
                            z3zE3413 = z2zE3379.ztup0;
                            struct zRlpCursorFields z1zE66;
                            z1zE66 = z2zE3379.ztup1;
                            struct ztuple_z8z5structz0zzRlpFieldRefFieldszCz0z5structz0zzRlpCursorFieldsz9 z2zE3380;
                            {
                              z2zE3380 = zrlp_cursor_pop(z1zE66);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE3205);
                                KILL(zTransaction)(&z3zE3207);
                                KILL(zTransaction)(&z3zE3208);
                                KILL(zTransaction)(&z3zE3209);
                                KILL(zTransaction)(&z3zE3210);
                                KILL(zTransaction)(&z3zE3211);
                                KILL(zTransaction)(&z3zE3212);
                                KILL(zTransaction)(&z3zE3213);
                                KILL(zTransaction)(&z3zE3214);
                                KILL(zTransaction)(&z3zE3215);
                                KILL(zTransaction)(&z3zE3216);
                                KILL(zTransaction)(&z3zE3217);
                                KILL(zTransaction)(&z3zE3218);
                                goto end_block_exception_3410;
                              }
                            }
                            struct zTransaction z3zE3219;
                            CREATE(zTransaction)(&z3zE3219);
                            {
                              struct zRlpFieldRefFields z3zE3414;
                              z3zE3414 = z2zE3380.ztup0;
                              struct zRlpCursorFields z1zE67;
                              z1zE67 = z2zE3380.ztup1;
                              unit z3zE3220;
                              {
                                z3zE3220 = zrlp_cursor_expect_end(z1zE67);
                                if (have_exception) {
                                  KILL(zTransaction)(&z3zE3205);
                                  KILL(zTransaction)(&z3zE3207);
                                  KILL(zTransaction)(&z3zE3208);
                                  KILL(zTransaction)(&z3zE3209);
                                  KILL(zTransaction)(&z3zE3210);
                                  KILL(zTransaction)(&z3zE3211);
                                  KILL(zTransaction)(&z3zE3212);
                                  KILL(zTransaction)(&z3zE3213);
                                  KILL(zTransaction)(&z3zE3214);
                                  KILL(zTransaction)(&z3zE3215);
                                  KILL(zTransaction)(&z3zE3216);
                                  KILL(zTransaction)(&z3zE3217);
                                  KILL(zTransaction)(&z3zE3218);
                                  KILL(zTransaction)(&z3zE3219);
                                  goto end_block_exception_3410;
                                }
                              }
                              sail_u256 z3zE3415;
                              {
                                z3zE3415 = zrlp_ref_word(z3zE3412);
                                if (have_exception) {
                                  KILL(zTransaction)(&z3zE3205);
                                  KILL(zTransaction)(&z3zE3207);
                                  KILL(zTransaction)(&z3zE3208);
                                  KILL(zTransaction)(&z3zE3209);
                                  KILL(zTransaction)(&z3zE3210);
                                  KILL(zTransaction)(&z3zE3211);
                                  KILL(zTransaction)(&z3zE3212);
                                  KILL(zTransaction)(&z3zE3213);
                                  KILL(zTransaction)(&z3zE3214);
                                  KILL(zTransaction)(&z3zE3215);
                                  KILL(zTransaction)(&z3zE3216);
                                  KILL(zTransaction)(&z3zE3217);
                                  KILL(zTransaction)(&z3zE3218);
                                  KILL(zTransaction)(&z3zE3219);
                                  goto end_block_exception_3410;
                                }
                              }
                              struct ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9 z2zE3381;
                              CREATE(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3381);
                              {
                                zdecode_access_list(&z2zE3381, z3zE3411);
                                if (have_exception) {
                                  KILL(zTransaction)(&z3zE3205);
                                  KILL(zTransaction)(&z3zE3207);
                                  KILL(zTransaction)(&z3zE3208);
                                  KILL(zTransaction)(&z3zE3209);
                                  KILL(zTransaction)(&z3zE3210);
                                  KILL(zTransaction)(&z3zE3211);
                                  KILL(zTransaction)(&z3zE3212);
                                  KILL(zTransaction)(&z3zE3213);
                                  KILL(zTransaction)(&z3zE3214);
                                  KILL(zTransaction)(&z3zE3215);
                                  KILL(zTransaction)(&z3zE3216);
                                  KILL(zTransaction)(&z3zE3217);
                                  KILL(zTransaction)(&z3zE3218);
                                  KILL(zTransaction)(&z3zE3219);
                                  KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3381);
                                  goto end_block_exception_3410;
                                }
                              }
                              struct zTransaction z3zE3221;
                              CREATE(zTransaction)(&z3zE3221);
                              {
                                zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z3zE3416;
                                CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3416);
                                COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3416, z2zE3381.ztup0);
                                zz5listz8z5structz0zzStorageKeyz9 z3zE3417;
                                CREATE(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3417);
                                COPY(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3417, z2zE3381.ztup1);
                                uint64_t z3zE3418;
                                z3zE3418 = z2zE3381.ztup2;
                                uint64_t z3zE3419;
                                z3zE3419 = z2zE3381.ztup3;
                                struct ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9 z2zE3382;
                                CREATE(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE3382);
                                {
                                  zdecode_auth_list(&z2zE3382, zauth_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE3205);
                                    KILL(zTransaction)(&z3zE3207);
                                    KILL(zTransaction)(&z3zE3208);
                                    KILL(zTransaction)(&z3zE3209);
                                    KILL(zTransaction)(&z3zE3210);
                                    KILL(zTransaction)(&z3zE3211);
                                    KILL(zTransaction)(&z3zE3212);
                                    KILL(zTransaction)(&z3zE3213);
                                    KILL(zTransaction)(&z3zE3214);
                                    KILL(zTransaction)(&z3zE3215);
                                    KILL(zTransaction)(&z3zE3216);
                                    KILL(zTransaction)(&z3zE3217);
                                    KILL(zTransaction)(&z3zE3218);
                                    KILL(zTransaction)(&z3zE3219);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3381);
                                    KILL(zTransaction)(&z3zE3221);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3416);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3417);
                                    KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE3382);
                                    goto end_block_exception_3410;
                                  }
                                }
                                struct zTransaction z3zE3222;
                                CREATE(zTransaction)(&z3zE3222);
                                {
                                  zz5listz8z5structz0zzAuthorizzzzationz9 zauthorizzations;
                                  CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                  COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations, z2zE3382.ztup0);
                                  uint64_t zauthorizzation_count;
                                  zauthorizzation_count = z2zE3382.ztup1;
                                  uint64_t z2zE3384;
                                  {
                                    uint64_t z2zE3383;
                                    {
                                      z2zE3383 = zrlp_ref_account_nonce(z3zE3404);
                                      if (have_exception) {
                                        KILL(zTransaction)(&z3zE3205);
                                        KILL(zTransaction)(&z3zE3207);
                                        KILL(zTransaction)(&z3zE3208);
                                        KILL(zTransaction)(&z3zE3209);
                                        KILL(zTransaction)(&z3zE3210);
                                        KILL(zTransaction)(&z3zE3211);
                                        KILL(zTransaction)(&z3zE3212);
                                        KILL(zTransaction)(&z3zE3213);
                                        KILL(zTransaction)(&z3zE3214);
                                        KILL(zTransaction)(&z3zE3215);
                                        KILL(zTransaction)(&z3zE3216);
                                        KILL(zTransaction)(&z3zE3217);
                                        KILL(zTransaction)(&z3zE3218);
                                        KILL(zTransaction)(&z3zE3219);
                                        KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3381);
                                        KILL(zTransaction)(&z3zE3221);
                                        KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3416);
                                        KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3417);
                                        KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE3382);
                                        KILL(zTransaction)(&z3zE3222);
                                        KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                        goto end_block_exception_3410;
                                      }
                                    }
                                    z2zE3384 = zword_of_account_nonce(z2zE3383);
                                  }
                                  uint64_t z2zE3385;
                                  {
                                    z2zE3385 = zrlp_ref_chain_identifier(z3zE3403);
                                    if (have_exception) {
                                      KILL(zTransaction)(&z3zE3205);
                                      KILL(zTransaction)(&z3zE3207);
                                      KILL(zTransaction)(&z3zE3208);
                                      KILL(zTransaction)(&z3zE3209);
                                      KILL(zTransaction)(&z3zE3210);
                                      KILL(zTransaction)(&z3zE3211);
                                      KILL(zTransaction)(&z3zE3212);
                                      KILL(zTransaction)(&z3zE3213);
                                      KILL(zTransaction)(&z3zE3214);
                                      KILL(zTransaction)(&z3zE3215);
                                      KILL(zTransaction)(&z3zE3216);
                                      KILL(zTransaction)(&z3zE3217);
                                      KILL(zTransaction)(&z3zE3218);
                                      KILL(zTransaction)(&z3zE3219);
                                      KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3381);
                                      KILL(zTransaction)(&z3zE3221);
                                      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3416);
                                      KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3417);
                                      KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE3382);
                                      KILL(zTransaction)(&z3zE3222);
                                      KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                      goto end_block_exception_3410;
                                    }
                                  }
                                  uint64_t z2zE3386;
                                  {
                                    z2zE3386 = zrlp_ref_gas(z3zE3407, zfork);
                                    if (have_exception) {
                                      KILL(zTransaction)(&z3zE3205);
                                      KILL(zTransaction)(&z3zE3207);
                                      KILL(zTransaction)(&z3zE3208);
                                      KILL(zTransaction)(&z3zE3209);
                                      KILL(zTransaction)(&z3zE3210);
                                      KILL(zTransaction)(&z3zE3211);
                                      KILL(zTransaction)(&z3zE3212);
                                      KILL(zTransaction)(&z3zE3213);
                                      KILL(zTransaction)(&z3zE3214);
                                      KILL(zTransaction)(&z3zE3215);
                                      KILL(zTransaction)(&z3zE3216);
                                      KILL(zTransaction)(&z3zE3217);
                                      KILL(zTransaction)(&z3zE3218);
                                      KILL(zTransaction)(&z3zE3219);
                                      KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3381);
                                      KILL(zTransaction)(&z3zE3221);
                                      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3416);
                                      KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3417);
                                      KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE3382);
                                      KILL(zTransaction)(&z3zE3222);
                                      KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                      goto end_block_exception_3410;
                                    }
                                  }
                                  bool z2zE3388;
                                  {
                                    uint64_t z2zE3387;
                                    z2zE3387 = z3zE3408.zcontent_len;
                                    z2zE3388 = (z2zE3387 == UINT64_C(0));
                                  }
                                  sail_fixed_bytes_20 z2zE3390;
                                  {
                                    sail_u256 z2zE3389;
                                    {
                                      z2zE3389 = zrlp_ref_word(z3zE3408);
                                      if (have_exception) {
                                        KILL(zTransaction)(&z3zE3205);
                                        KILL(zTransaction)(&z3zE3207);
                                        KILL(zTransaction)(&z3zE3208);
                                        KILL(zTransaction)(&z3zE3209);
                                        KILL(zTransaction)(&z3zE3210);
                                        KILL(zTransaction)(&z3zE3211);
                                        KILL(zTransaction)(&z3zE3212);
                                        KILL(zTransaction)(&z3zE3213);
                                        KILL(zTransaction)(&z3zE3214);
                                        KILL(zTransaction)(&z3zE3215);
                                        KILL(zTransaction)(&z3zE3216);
                                        KILL(zTransaction)(&z3zE3217);
                                        KILL(zTransaction)(&z3zE3218);
                                        KILL(zTransaction)(&z3zE3219);
                                        KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3381);
                                        KILL(zTransaction)(&z3zE3221);
                                        KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3416);
                                        KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3417);
                                        KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE3382);
                                        KILL(zTransaction)(&z3zE3222);
                                        KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                        goto end_block_exception_3410;
                                      }
                                    }
                                    z2zE3390 = zword_to_address(z2zE3389);
                                  }
                                  sail_u256 z2zE3391;
                                  {
                                    z2zE3391 = zrlp_ref_uint_word(z3zE3409);
                                    if (have_exception) {
                                      KILL(zTransaction)(&z3zE3205);
                                      KILL(zTransaction)(&z3zE3207);
                                      KILL(zTransaction)(&z3zE3208);
                                      KILL(zTransaction)(&z3zE3209);
                                      KILL(zTransaction)(&z3zE3210);
                                      KILL(zTransaction)(&z3zE3211);
                                      KILL(zTransaction)(&z3zE3212);
                                      KILL(zTransaction)(&z3zE3213);
                                      KILL(zTransaction)(&z3zE3214);
                                      KILL(zTransaction)(&z3zE3215);
                                      KILL(zTransaction)(&z3zE3216);
                                      KILL(zTransaction)(&z3zE3217);
                                      KILL(zTransaction)(&z3zE3218);
                                      KILL(zTransaction)(&z3zE3219);
                                      KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3381);
                                      KILL(zTransaction)(&z3zE3221);
                                      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3416);
                                      KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3417);
                                      KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE3382);
                                      KILL(zTransaction)(&z3zE3222);
                                      KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                      goto end_block_exception_3410;
                                    }
                                  }
                                  struct zByteSliceFields z2zE3392;
                                  {
                                    z2zE3392 = ztx_input_span(z3zE3410);
                                    if (have_exception) {
                                      KILL(zTransaction)(&z3zE3205);
                                      KILL(zTransaction)(&z3zE3207);
                                      KILL(zTransaction)(&z3zE3208);
                                      KILL(zTransaction)(&z3zE3209);
                                      KILL(zTransaction)(&z3zE3210);
                                      KILL(zTransaction)(&z3zE3211);
                                      KILL(zTransaction)(&z3zE3212);
                                      KILL(zTransaction)(&z3zE3213);
                                      KILL(zTransaction)(&z3zE3214);
                                      KILL(zTransaction)(&z3zE3215);
                                      KILL(zTransaction)(&z3zE3216);
                                      KILL(zTransaction)(&z3zE3217);
                                      KILL(zTransaction)(&z3zE3218);
                                      KILL(zTransaction)(&z3zE3219);
                                      KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3381);
                                      KILL(zTransaction)(&z3zE3221);
                                      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3416);
                                      KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3417);
                                      KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE3382);
                                      KILL(zTransaction)(&z3zE3222);
                                      KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                      goto end_block_exception_3410;
                                    }
                                  }
                                  sail_u256 z2zE3393;
                                  {
                                    z2zE3393 = zrlp_ref_uint_word(z3zE3406);
                                    if (have_exception) {
                                      KILL(zTransaction)(&z3zE3205);
                                      KILL(zTransaction)(&z3zE3207);
                                      KILL(zTransaction)(&z3zE3208);
                                      KILL(zTransaction)(&z3zE3209);
                                      KILL(zTransaction)(&z3zE3210);
                                      KILL(zTransaction)(&z3zE3211);
                                      KILL(zTransaction)(&z3zE3212);
                                      KILL(zTransaction)(&z3zE3213);
                                      KILL(zTransaction)(&z3zE3214);
                                      KILL(zTransaction)(&z3zE3215);
                                      KILL(zTransaction)(&z3zE3216);
                                      KILL(zTransaction)(&z3zE3217);
                                      KILL(zTransaction)(&z3zE3218);
                                      KILL(zTransaction)(&z3zE3219);
                                      KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3381);
                                      KILL(zTransaction)(&z3zE3221);
                                      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3416);
                                      KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3417);
                                      KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE3382);
                                      KILL(zTransaction)(&z3zE3222);
                                      KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                      goto end_block_exception_3410;
                                    }
                                  }
                                  sail_u256 z2zE3394;
                                  {
                                    z2zE3394 = zrlp_ref_uint_word(z3zE3405);
                                    if (have_exception) {
                                      KILL(zTransaction)(&z3zE3205);
                                      KILL(zTransaction)(&z3zE3207);
                                      KILL(zTransaction)(&z3zE3208);
                                      KILL(zTransaction)(&z3zE3209);
                                      KILL(zTransaction)(&z3zE3210);
                                      KILL(zTransaction)(&z3zE3211);
                                      KILL(zTransaction)(&z3zE3212);
                                      KILL(zTransaction)(&z3zE3213);
                                      KILL(zTransaction)(&z3zE3214);
                                      KILL(zTransaction)(&z3zE3215);
                                      KILL(zTransaction)(&z3zE3216);
                                      KILL(zTransaction)(&z3zE3217);
                                      KILL(zTransaction)(&z3zE3218);
                                      KILL(zTransaction)(&z3zE3219);
                                      KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3381);
                                      KILL(zTransaction)(&z3zE3221);
                                      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3416);
                                      KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3417);
                                      KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE3382);
                                      KILL(zTransaction)(&z3zE3222);
                                      KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                      goto end_block_exception_3410;
                                    }
                                  }
                                  sail_fixed_bytes_32 z2zE3396;
                                  {
                                    struct zByteSliceFields z2zE3395;
                                    {
                                      z2zE3395 = ztx_sig_span(z3zE3403, z3zE3412);
                                      if (have_exception) {
                                        KILL(zTransaction)(&z3zE3205);
                                        KILL(zTransaction)(&z3zE3207);
                                        KILL(zTransaction)(&z3zE3208);
                                        KILL(zTransaction)(&z3zE3209);
                                        KILL(zTransaction)(&z3zE3210);
                                        KILL(zTransaction)(&z3zE3211);
                                        KILL(zTransaction)(&z3zE3212);
                                        KILL(zTransaction)(&z3zE3213);
                                        KILL(zTransaction)(&z3zE3214);
                                        KILL(zTransaction)(&z3zE3215);
                                        KILL(zTransaction)(&z3zE3216);
                                        KILL(zTransaction)(&z3zE3217);
                                        KILL(zTransaction)(&z3zE3218);
                                        KILL(zTransaction)(&z3zE3219);
                                        KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3381);
                                        KILL(zTransaction)(&z3zE3221);
                                        KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3416);
                                        KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3417);
                                        KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE3382);
                                        KILL(zTransaction)(&z3zE3222);
                                        KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                        goto end_block_exception_3410;
                                      }
                                    }
                                    {
                                      z2zE3396 = ztx_signing_hash(zSetCodeTx, z2zE3395, z3zE3415);
                                      if (have_exception) {
                                        KILL(zTransaction)(&z3zE3205);
                                        KILL(zTransaction)(&z3zE3207);
                                        KILL(zTransaction)(&z3zE3208);
                                        KILL(zTransaction)(&z3zE3209);
                                        KILL(zTransaction)(&z3zE3210);
                                        KILL(zTransaction)(&z3zE3211);
                                        KILL(zTransaction)(&z3zE3212);
                                        KILL(zTransaction)(&z3zE3213);
                                        KILL(zTransaction)(&z3zE3214);
                                        KILL(zTransaction)(&z3zE3215);
                                        KILL(zTransaction)(&z3zE3216);
                                        KILL(zTransaction)(&z3zE3217);
                                        KILL(zTransaction)(&z3zE3218);
                                        KILL(zTransaction)(&z3zE3219);
                                        KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3381);
                                        KILL(zTransaction)(&z3zE3221);
                                        KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3416);
                                        KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3417);
                                        KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE3382);
                                        KILL(zTransaction)(&z3zE3222);
                                        KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                        goto end_block_exception_3410;
                                      }
                                    }
                                  }
                                  sail_u256 z2zE3397;
                                  {
                                    z2zE3397 = zrlp_ref_uint_word(z3zE3413);
                                    if (have_exception) {
                                      KILL(zTransaction)(&z3zE3205);
                                      KILL(zTransaction)(&z3zE3207);
                                      KILL(zTransaction)(&z3zE3208);
                                      KILL(zTransaction)(&z3zE3209);
                                      KILL(zTransaction)(&z3zE3210);
                                      KILL(zTransaction)(&z3zE3211);
                                      KILL(zTransaction)(&z3zE3212);
                                      KILL(zTransaction)(&z3zE3213);
                                      KILL(zTransaction)(&z3zE3214);
                                      KILL(zTransaction)(&z3zE3215);
                                      KILL(zTransaction)(&z3zE3216);
                                      KILL(zTransaction)(&z3zE3217);
                                      KILL(zTransaction)(&z3zE3218);
                                      KILL(zTransaction)(&z3zE3219);
                                      KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3381);
                                      KILL(zTransaction)(&z3zE3221);
                                      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3416);
                                      KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3417);
                                      KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE3382);
                                      KILL(zTransaction)(&z3zE3222);
                                      KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                      goto end_block_exception_3410;
                                    }
                                  }
                                  sail_u256 z2zE3398;
                                  {
                                    z2zE3398 = zrlp_ref_uint_word(z3zE3414);
                                    if (have_exception) {
                                      KILL(zTransaction)(&z3zE3205);
                                      KILL(zTransaction)(&z3zE3207);
                                      KILL(zTransaction)(&z3zE3208);
                                      KILL(zTransaction)(&z3zE3209);
                                      KILL(zTransaction)(&z3zE3210);
                                      KILL(zTransaction)(&z3zE3211);
                                      KILL(zTransaction)(&z3zE3212);
                                      KILL(zTransaction)(&z3zE3213);
                                      KILL(zTransaction)(&z3zE3214);
                                      KILL(zTransaction)(&z3zE3215);
                                      KILL(zTransaction)(&z3zE3216);
                                      KILL(zTransaction)(&z3zE3217);
                                      KILL(zTransaction)(&z3zE3218);
                                      KILL(zTransaction)(&z3zE3219);
                                      KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3381);
                                      KILL(zTransaction)(&z3zE3221);
                                      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3416);
                                      KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3417);
                                      KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE3382);
                                      KILL(zTransaction)(&z3zE3222);
                                      KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                      goto end_block_exception_3410;
                                    }
                                  }
                                  z3zE3222.zaccess_list_address_count = z3zE3418;
                                  COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE3222)->zaccess_list_addresses), z3zE3416);
                                  z3zE3222.zaccess_list_slot_count = z3zE3419;
                                  COPY(zz5listz8z5structz0zzStorageKeyz9)(&((&z3zE3222)->zaccess_list_slots), z3zE3417);
                                  z3zE3222.zauthorizzation_count = zauthorizzation_count;
                                  COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&((&z3zE3222)->zauthorizzations), zauthorizzations);
                                  z3zE3222.zblob_hashes = zEMPTY_BLOB_HASHES;
                                  z3zE3222.zchain_id = z2zE3385;
                                  z3zE3222.zgas_limit = z2zE3386;
                                  z3zE3222.zinput_src = z2zE3392;
                                  z3zE3222.zis_create = z2zE3388;
                                  z3zE3222.zmax_blob_fee = zZERO_WORD;
                                  z3zE3222.zmax_fee = z2zE3393;
                                  z3zE3222.zmax_priority_fee = z2zE3394;
                                  z3zE3222.znonce = u256_of_fbits(z2zE3384);
                                  z3zE3222.zpubkey = zpubkey;
                                  z3zE3222.zraw = ztx;
                                  z3zE3222.zrecipient = z2zE3390;
                                  z3zE3222.zsender = zsender;
                                  z3zE3222.zsig_r = z2zE3397;
                                  z3zE3222.zsig_s = z2zE3398;
                                  z3zE3222.zsig_v = z3zE3415;
                                  z3zE3222.zsigning_hash = z2zE3396;
                                  z3zE3222.ztx_type = zSetCodeTx;
                                  z3zE3222.zvalue = z2zE3391;
                                  KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                  goto finish_match_3305;
                                }
                              case_3306: ;
                                sail_match_failure("rlp_decode_tx");
                              finish_match_3305: ;
                                COPY(zTransaction)(&z3zE3221, z3zE3222);
                                KILL(zTransaction)(&z3zE3222);
                                KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE3382);
                                KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE3417);
                                KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE3416);
                                goto finish_match_3303;
                              }
                            case_3304: ;
                              sail_match_failure("rlp_decode_tx");
                            finish_match_3303: ;
                              COPY(zTransaction)(&z3zE3219, z3zE3221);
                              KILL(zTransaction)(&z3zE3221);
                              KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE3381);
                              goto finish_match_3301;
                            }
                          case_3302: ;
                            sail_match_failure("rlp_decode_tx");
                          finish_match_3301: ;
                            COPY(zTransaction)(&z3zE3218, z3zE3219);
                            KILL(zTransaction)(&z3zE3219);
                            goto finish_match_3299;
                          }
                        case_3300: ;
                          sail_match_failure("rlp_decode_tx");
                        finish_match_3299: ;
                          COPY(zTransaction)(&z3zE3217, z3zE3218);
                          KILL(zTransaction)(&z3zE3218);
                          goto finish_match_3297;
                        }
                      case_3298: ;
                        sail_match_failure("rlp_decode_tx");
                      finish_match_3297: ;
                        COPY(zTransaction)(&z3zE3216, z3zE3217);
                        KILL(zTransaction)(&z3zE3217);
                        goto finish_match_3295;
                      }
                    case_3296: ;
                      sail_match_failure("rlp_decode_tx");
                    finish_match_3295: ;
                      COPY(zTransaction)(&z3zE3215, z3zE3216);
                      KILL(zTransaction)(&z3zE3216);
                      goto finish_match_3293;
                    }
                  case_3294: ;
                    sail_match_failure("rlp_decode_tx");
                  finish_match_3293: ;
                    COPY(zTransaction)(&z3zE3214, z3zE3215);
                    KILL(zTransaction)(&z3zE3215);
                    goto finish_match_3291;
                  }
                case_3292: ;
                  sail_match_failure("rlp_decode_tx");
                finish_match_3291: ;
                  COPY(zTransaction)(&z3zE3213, z3zE3214);
                  KILL(zTransaction)(&z3zE3214);
                  goto finish_match_3289;
                }
              case_3290: ;
                sail_match_failure("rlp_decode_tx");
              finish_match_3289: ;
                COPY(zTransaction)(&z3zE3212, z3zE3213);
                KILL(zTransaction)(&z3zE3213);
                goto finish_match_3287;
              }
            case_3288: ;
              sail_match_failure("rlp_decode_tx");
            finish_match_3287: ;
              COPY(zTransaction)(&z3zE3211, z3zE3212);
              KILL(zTransaction)(&z3zE3212);
              goto finish_match_3285;
            }
          case_3286: ;
            sail_match_failure("rlp_decode_tx");
          finish_match_3285: ;
            COPY(zTransaction)(&z3zE3210, z3zE3211);
            KILL(zTransaction)(&z3zE3211);
            goto finish_match_3283;
          }
        case_3284: ;
          sail_match_failure("rlp_decode_tx");
        finish_match_3283: ;
          COPY(zTransaction)(&z3zE3209, z3zE3210);
          KILL(zTransaction)(&z3zE3210);
          goto finish_match_3281;
        }
      case_3282: ;
        sail_match_failure("rlp_decode_tx");
      finish_match_3281: ;
        COPY(zTransaction)(&z3zE3208, z3zE3209);
        KILL(zTransaction)(&z3zE3209);
        goto finish_match_3279;
      }
    case_3280: ;
      sail_match_failure("rlp_decode_tx");
    finish_match_3279: ;
      COPY(zTransaction)(&z3zE3207, z3zE3208);
      KILL(zTransaction)(&z3zE3208);
      goto finish_match_3277;
    }
  case_3278: ;
    sail_match_failure("rlp_decode_tx");
  finish_match_3277: ;
    COPY(zTransaction)(&z3zE3205, z3zE3207);
    KILL(zTransaction)(&z3zE3207);
    goto finish_match_3274;
  }
case_3276: ;
  {
    struct zexception z2zE3399;
    CREATE(zexception)(&z2zE3399);
    zInvalidBlock(&z2zE3399, zRlpDecode);
    COPY(zexception)(current_exception, z2zE3399);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/tx.sail:631.13-631.42");
    KILL(zTransaction)(&z3zE3205);
    KILL(zexception)(&z2zE3399);
    goto end_block_exception_3410;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3399);
    goto finish_match_3274;
  }
case_3275: ;
finish_match_3274: ;
  COPY(zTransaction)((*(&z8zE208)), z3zE3205);
  KILL(zTransaction)(&z3zE3205);
end_function_3409: ;
  goto end_function_4078;
end_block_exception_3410: ;
  goto end_function_4078;
end_function_4078: ;
}

