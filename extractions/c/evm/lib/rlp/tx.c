/* Generated from sail/lib/rlp/tx.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../../evm_internal.h"
#endif
void create_letbind_114(void) {    CREATE(zAccessListDecode)(&zEMPTY_ACCESS_LIST_DECODE);

  struct zAccessListDecode z3zE128;
  CREATE(zAccessListDecode)(&z3zE128);
  struct zAccessListDecode z3zE125;
  CREATE(zAccessListDecode)(&z3zE125);
  z3zE125.zaddress_count = UINT64_C(0);
  zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z3zE126;
  CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE126);
  COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE125)->zaddresses), z3zE126);
  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE126);
  z3zE125.zslot_count = UINT64_C(0);
  zz5listz8z5structz0zzStorageKeyz9 z3zE127;
  CREATE(zz5listz8z5structz0zzStorageKeyz9)(&z3zE127);
  COPY(zz5listz8z5structz0zzStorageKeyz9)(&((&z3zE125)->zstorage_slots), z3zE127);
  KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE127);
  COPY(zAccessListDecode)(&z3zE128, z3zE125);
  KILL(zAccessListDecode)(&z3zE125);
  COPY(zAccessListDecode)(&zEMPTY_ACCESS_LIST_DECODE, z3zE128);
  KILL(zAccessListDecode)(&z3zE128);
let_end_312: ;
}
void kill_letbind_114(void) {    KILL(zAccessListDecode)(&zEMPTY_ACCESS_LIST_DECODE);
}

void zdecode_access_list_keys(struct zAccessListDecode *z8zE186, struct zByteSliceFields zcursor, sail_fixed_bytes_20 zaddr, struct zAccessListDecode ztail)
{
  bool z2zE2773;
  {
    uint64_t z2zE2772;
    z2zE2772 = zcursor.zlen;
    z2zE2773 = (z2zE2772 == UINT64_C(0));
  }
  if (z2zE2773) {
    zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE2774;
    CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE2774);
    COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE2774, ztail.zaddresses);
    zz5listz8z5structz0zzStorageKeyz9 z2zE2775;
    CREATE(zz5listz8z5structz0zzStorageKeyz9)(&z2zE2775);
    COPY(zz5listz8z5structz0zzStorageKeyz9)(&z2zE2775, ztail.zstorage_slots);
    uint64_t z2zE2776;
    z2zE2776 = ztail.zaddress_count;
    uint64_t z2zE2777;
    z2zE2777 = ztail.zslot_count;
    struct zAccessListDecode z3zE2637;
    CREATE(zAccessListDecode)(&z3zE2637);
    z3zE2637.zaddress_count = z2zE2776;
    COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE2637)->zaddresses), z2zE2774);
    z3zE2637.zslot_count = z2zE2777;
    COPY(zz5listz8z5structz0zzStorageKeyz9)(&((&z3zE2637)->zstorage_slots), z2zE2775);
    COPY(zAccessListDecode)((*(&z8zE186)), z3zE2637);
    KILL(zAccessListDecode)(&z3zE2637);
    KILL(zz5listz8z5structz0zzStorageKeyz9)(&z2zE2775);
    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE2774);
  } else {
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2778;
    {
      z2zE2778 = zrlp_cursor_pop(zcursor);
      if (have_exception) {  goto end_block_exception_3057;  }
    }
    struct zAccessListDecode z3zE2633;
    CREATE(zAccessListDecode)(&z3zE2633);
    {
      struct zRlpFieldRef zkey;
      zkey = z2zE2778.ztup0;
      struct zByteSliceFields znext;
      znext = z2zE2778.ztup1;
      struct zStorageKey zstorage_key;
      {
        sail_u256 z2zE2785;
        {
          z2zE2785 = zrlp_ref_word(zkey);
          if (have_exception) {
            KILL(zAccessListDecode)(&z3zE2633);
            goto end_block_exception_3057;
          }
        }
        struct zStorageKey z3zE2634;
        z3zE2634.zaddr = zaddr;
        z3zE2634.zslot = z2zE2785;
        zstorage_key = z3zE2634;
      }
      struct zAccessListDecode zresult;
      CREATE(zAccessListDecode)(&zresult);
      {
        zdecode_access_list_keys(&zresult, znext, zaddr, ztail);
        if (have_exception) {
          KILL(zAccessListDecode)(&z3zE2633);
          KILL(zAccessListDecode)(&zresult);
          goto end_block_exception_3057;
        }
      }
      zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE2779;
      CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE2779);
      COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE2779, zresult.zaddresses);
      zz5listz8z5structz0zzStorageKeyz9 z2zE2781;
      CREATE(zz5listz8z5structz0zzStorageKeyz9)(&z2zE2781);
      {
        zz5listz8z5structz0zzStorageKeyz9 z2zE2780;
        CREATE(zz5listz8z5structz0zzStorageKeyz9)(&z2zE2780);
        COPY(zz5listz8z5structz0zzStorageKeyz9)(&z2zE2780, zresult.zstorage_slots);
        zconsz3z5structz0zzStorageKey(&z2zE2781, zstorage_key, z2zE2780);
        KILL(zz5listz8z5structz0zzStorageKeyz9)(&z2zE2780);
      }
      uint64_t z2zE2782;
      z2zE2782 = zresult.zaddress_count;
      uint64_t z2zE2784;
      {
        uint64_t z2zE2783;
        z2zE2783 = zresult.zslot_count;
        z2zE2784 = zrlp_item_count_incrementzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2783);
      }
      struct zAccessListDecode z3zE2635;
      CREATE(zAccessListDecode)(&z3zE2635);
      z3zE2635.zaddress_count = z2zE2782;
      COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE2635)->zaddresses), z2zE2779);
      z3zE2635.zslot_count = z2zE2784;
      COPY(zz5listz8z5structz0zzStorageKeyz9)(&((&z3zE2635)->zstorage_slots), z2zE2781);
      COPY(zAccessListDecode)(&z3zE2633, z3zE2635);
      KILL(zAccessListDecode)(&z3zE2635);
      KILL(zz5listz8z5structz0zzStorageKeyz9)(&z2zE2781);
      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE2779);
      KILL(zAccessListDecode)(&zresult);
      goto finish_match_3054;
    }
  case_3055: ;
    sail_match_failure("decode_access_list_keys");
  finish_match_3054: ;
    COPY(zAccessListDecode)((*(&z8zE186)), z3zE2633);
    KILL(zAccessListDecode)(&z3zE2633);
  }
end_function_3056: ;
  goto end_function_3652;
end_block_exception_3057: ;
  goto end_function_3652;
end_function_3652: ;
}

void zdecode_access_list_entries(struct zAccessListDecode *z8zE187, struct zByteSliceFields zcursor)
{
  bool z2zE2759;
  {
    uint64_t z2zE2758;
    z2zE2758 = zcursor.zlen;
    z2zE2759 = (z2zE2758 == UINT64_C(0));
  }
  if (z2zE2759) {
    struct zAccessListDecode z3zE2630;
    CREATE(zAccessListDecode)(&z3zE2630);
    z3zE2630.zaddress_count = UINT64_C(0);
    zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z3zE2631;
    CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE2631);
    COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE2630)->zaddresses), z3zE2631);
    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE2631);
    z3zE2630.zslot_count = UINT64_C(0);
    zz5listz8z5structz0zzStorageKeyz9 z3zE2632;
    CREATE(zz5listz8z5structz0zzStorageKeyz9)(&z3zE2632);
    COPY(zz5listz8z5structz0zzStorageKeyz9)(&((&z3zE2630)->zstorage_slots), z3zE2632);
    KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE2632);
    COPY(zAccessListDecode)((*(&z8zE187)), z3zE2630);
    KILL(zAccessListDecode)(&z3zE2630);
  } else {
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2760;
    {
      z2zE2760 = zrlp_cursor_pop(zcursor);
      if (have_exception) {  goto end_block_exception_3053;  }
    }
    struct zAccessListDecode z3zE2622;
    CREATE(zAccessListDecode)(&z3zE2622);
    {
      struct zRlpFieldRef zentry;
      zentry = z2zE2760.ztup0;
      struct zByteSliceFields znext;
      znext = z2zE2760.ztup1;
      struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2762;
      {
        struct zByteSliceFields z2zE2761;
        {
          z2zE2761 = zrlp_ref_cursor(zentry);
          if (have_exception) {
            KILL(zAccessListDecode)(&z3zE2622);
            goto end_block_exception_3053;
          }
        }
        {
          z2zE2762 = zrlp_cursor_pop(z2zE2761);
          if (have_exception) {
            KILL(zAccessListDecode)(&z3zE2622);
            goto end_block_exception_3053;
          }
        }
      }
      struct zAccessListDecode z3zE2623;
      CREATE(zAccessListDecode)(&z3zE2623);
      {
        struct zRlpFieldRef zaddr_f;
        zaddr_f = z2zE2762.ztup0;
        struct zByteSliceFields zentry_fields;
        zentry_fields = z2zE2762.ztup1;
        struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2763;
        {
          z2zE2763 = zrlp_cursor_pop(zentry_fields);
          if (have_exception) {
            KILL(zAccessListDecode)(&z3zE2622);
            KILL(zAccessListDecode)(&z3zE2623);
            goto end_block_exception_3053;
          }
        }
        struct zAccessListDecode z3zE2624;
        CREATE(zAccessListDecode)(&z3zE2624);
        {
          struct zRlpFieldRef zkeys_f;
          zkeys_f = z2zE2763.ztup0;
          struct zByteSliceFields z1zE89;
          z1zE89 = z2zE2763.ztup1;
          unit z3zE2625;
          {
            z3zE2625 = zrlp_cursor_expect_end(z1zE89);
            if (have_exception) {
              KILL(zAccessListDecode)(&z3zE2622);
              KILL(zAccessListDecode)(&z3zE2623);
              KILL(zAccessListDecode)(&z3zE2624);
              goto end_block_exception_3053;
            }
          }
          sail_fixed_bytes_20 zaddr;
          {
            sail_u256 z2zE2771;
            {
              z2zE2771 = zrlp_ref_word(zaddr_f);
              if (have_exception) {
                KILL(zAccessListDecode)(&z3zE2622);
                KILL(zAccessListDecode)(&z3zE2623);
                KILL(zAccessListDecode)(&z3zE2624);
                goto end_block_exception_3053;
              }
            }
            zaddr = evmsail_word_to_address(z2zE2771);
          }
          struct zAccessListDecode ztail;
          CREATE(zAccessListDecode)(&ztail);
          {
            zdecode_access_list_entries(&ztail, znext);
            if (have_exception) {
              KILL(zAccessListDecode)(&z3zE2622);
              KILL(zAccessListDecode)(&z3zE2623);
              KILL(zAccessListDecode)(&z3zE2624);
              KILL(zAccessListDecode)(&ztail);
              goto end_block_exception_3053;
            }
          }
          struct zAccessListDecode zresult;
          CREATE(zAccessListDecode)(&zresult);
          {
            struct zByteSliceFields z2zE2770;
            {
              z2zE2770 = zrlp_ref_cursor(zkeys_f);
              if (have_exception) {
                KILL(zAccessListDecode)(&z3zE2622);
                KILL(zAccessListDecode)(&z3zE2623);
                KILL(zAccessListDecode)(&z3zE2624);
                KILL(zAccessListDecode)(&ztail);
                KILL(zAccessListDecode)(&zresult);
                goto end_block_exception_3053;
              }
            }
            {
              zdecode_access_list_keys(&zresult, z2zE2770, zaddr, ztail);
              if (have_exception) {
                KILL(zAccessListDecode)(&z3zE2622);
                KILL(zAccessListDecode)(&z3zE2623);
                KILL(zAccessListDecode)(&z3zE2624);
                KILL(zAccessListDecode)(&ztail);
                KILL(zAccessListDecode)(&zresult);
                goto end_block_exception_3053;
              }
            }
          }
          zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE2765;
          CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE2765);
          {
            zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE2764;
            CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE2764);
            COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE2764, zresult.zaddresses);
            zconsz3z5structz0zz__sail_c_repr_fixed_bytesz820z9(&z2zE2765, zaddr, z2zE2764);
            KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE2764);
          }
          zz5listz8z5structz0zzStorageKeyz9 z2zE2766;
          CREATE(zz5listz8z5structz0zzStorageKeyz9)(&z2zE2766);
          COPY(zz5listz8z5structz0zzStorageKeyz9)(&z2zE2766, zresult.zstorage_slots);
          uint64_t z2zE2768;
          {
            uint64_t z2zE2767;
            z2zE2767 = zresult.zaddress_count;
            z2zE2768 = zrlp_item_count_incrementzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2767);
          }
          uint64_t z2zE2769;
          z2zE2769 = zresult.zslot_count;
          struct zAccessListDecode z3zE2626;
          CREATE(zAccessListDecode)(&z3zE2626);
          z3zE2626.zaddress_count = z2zE2768;
          COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE2626)->zaddresses), z2zE2765);
          z3zE2626.zslot_count = z2zE2769;
          COPY(zz5listz8z5structz0zzStorageKeyz9)(&((&z3zE2626)->zstorage_slots), z2zE2766);
          COPY(zAccessListDecode)(&z3zE2624, z3zE2626);
          KILL(zAccessListDecode)(&z3zE2626);
          KILL(zz5listz8z5structz0zzStorageKeyz9)(&z2zE2766);
          KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE2765);
          KILL(zAccessListDecode)(&zresult);
          KILL(zAccessListDecode)(&ztail);
          goto finish_match_3050;
        }
      case_3051: ;
        sail_match_failure("decode_access_list_entries");
      finish_match_3050: ;
        COPY(zAccessListDecode)(&z3zE2623, z3zE2624);
        KILL(zAccessListDecode)(&z3zE2624);
        goto finish_match_3048;
      }
    case_3049: ;
      sail_match_failure("decode_access_list_entries");
    finish_match_3048: ;
      COPY(zAccessListDecode)(&z3zE2622, z3zE2623);
      KILL(zAccessListDecode)(&z3zE2623);
      goto finish_match_3046;
    }
  case_3047: ;
    sail_match_failure("decode_access_list_entries");
  finish_match_3046: ;
    COPY(zAccessListDecode)((*(&z8zE187)), z3zE2622);
    KILL(zAccessListDecode)(&z3zE2622);
  }
end_function_3052: ;
  goto end_function_3651;
end_block_exception_3053: ;
  goto end_function_3651;
end_function_3651: ;
}

void zdecode_access_list(struct ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9 *z8zE188, struct zRlpFieldRef zf)
{
  bool z2zE2751;
  {
    uint64_t z2zE2750;
    {
      struct zByteSliceFields z2zE2749;
      z2zE2749 = zf.zsource;
      z2zE2750 = z2zE2749.zlen;
    }
    z2zE2751 = (!(UINT64_C(1073741824) < z2zE2750));
  }
  if (z2zE2751) {
    struct zAccessListDecode zdecoded;
    CREATE(zAccessListDecode)(&zdecoded);
    {
      struct zByteSliceFields z2zE2756;
      {
        z2zE2756 = zrlp_ref_cursor(zf);
        if (have_exception) {
          KILL(zAccessListDecode)(&zdecoded);
          goto end_block_exception_3045;
        }
      }
      {
        zdecode_access_list_entries(&zdecoded, z2zE2756);
        if (have_exception) {
          KILL(zAccessListDecode)(&zdecoded);
          goto end_block_exception_3045;
        }
      }
    }
    zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE2752;
    CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE2752);
    COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE2752, zdecoded.zaddresses);
    zz5listz8z5structz0zzStorageKeyz9 z2zE2753;
    CREATE(zz5listz8z5structz0zzStorageKeyz9)(&z2zE2753);
    COPY(zz5listz8z5structz0zzStorageKeyz9)(&z2zE2753, zdecoded.zstorage_slots);
    uint64_t z2zE2754;
    z2zE2754 = zdecoded.zaddress_count;
    uint64_t z2zE2755;
    z2zE2755 = zdecoded.zslot_count;
    struct ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9 z3zE2621;
    CREATE(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z3zE2621);
    COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE2621)->ztup0), z2zE2752);
    COPY(zz5listz8z5structz0zzStorageKeyz9)(&((&z3zE2621)->ztup1), z2zE2753);
    z3zE2621.ztup2 = z2zE2754;
    z3zE2621.ztup3 = z2zE2755;
    COPY(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)((*(&z8zE188)), z3zE2621);
    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z3zE2621);
    KILL(zz5listz8z5structz0zzStorageKeyz9)(&z2zE2753);
    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE2752);
    KILL(zAccessListDecode)(&zdecoded);
  } else {
    struct zexception z2zE2757;
    CREATE(zexception)(&z2zE2757);
    zInvalidBlock(&z2zE2757, zRlpDecode);
    COPY(zexception)(current_exception, z2zE2757);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/tx.sail:111.8-111.37");
    KILL(zexception)(&z2zE2757);
    goto end_block_exception_3045;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2757);
  }
end_function_3044: ;
  goto end_function_3650;
end_block_exception_3045: ;
  goto end_function_3650;
end_function_3650: ;
}

void create_letbind_115(void) {

  uint64_t z3zE129;
  z3zE129 = UINT64_C(33);
  zBLOB_HASH_RLP_LENGTH = z3zE129;
let_end_317: ;
}
void kill_letbind_115(void) {
}

void create_letbind_116(void) {

  uint64_t z3zE130;
  z3zE130 = zWORD_BYTE_LENGTH;
  zBLOB_HASH_LENGTH = z3zE130;
let_end_318: ;
}
void kill_letbind_116(void) {
}

uint64_t zdecode_blob_hash_items(struct zByteSliceFields zcursor, uint64_t zcount)
{
  uint64_t z8zE189;
  bool z2zE2734;
  {
    uint64_t z2zE2733;
    z2zE2733 = zcursor.zlen;
    z2zE2734 = (z2zE2733 == UINT64_C(0));
  }
  if (z2zE2734) {  z8zE189 = zcount;  } else {
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2735;
    {
      z2zE2735 = zrlp_cursor_pop(zcursor);
      if (have_exception) {  goto end_block_exception_3043;  }
    }
    uint64_t z3zE2615;
    {
      struct zRlpFieldRef zitem;
      zitem = z2zE2735.ztup0;
      struct zByteSliceFields znext;
      znext = z2zE2735.ztup1;
      bool z2zE2744;
      {
        bool z2zE2743;
        z2zE2743 = zitem.zis_list;
        bool z3zE2618;
        if (z2zE2743) {  z3zE2618 = true;  } else {
          bool z2zE2742;
          {
            uint64_t z2zE2737;
            {
              struct zByteSliceFields z2zE2736;
              z2zE2736 = zitem.zsource;
              z2zE2737 = z2zE2736.zlen;
            }
            z2zE2742 = (z2zE2737 != zBLOB_HASH_RLP_LENGTH);
          }
          bool z3zE2617;
          if (z2zE2742) {  z3zE2617 = true;  } else {
            bool z2zE2741;
            {
              uint64_t z2zE2738;
              z2zE2738 = zitem.zcontent_len;
              z2zE2741 = (z2zE2738 != zBLOB_HASH_LENGTH);
            }
            bool z3zE2616;
            if (z2zE2741) {  z3zE2616 = true;  } else {
              uint64_t z2zE2740;
              {
                struct zByteSliceFields z2zE2739;
                z2zE2739 = zitem.zsource;
                z2zE2740 = zslice_byte(z2zE2739, UINT64_C(0));
              }
              z3zE2616 = (z2zE2740 != UINT64_C(0xA0));
            }
            z3zE2617 = z3zE2616;
          }
          z3zE2618 = z3zE2617;
        }
        z2zE2744 = z3zE2618;
      }
      unit z3zE2619;
      if (z2zE2744) {
        struct zexception z2zE2745;
        CREATE(zexception)(&z2zE2745);
        zInvalidBlock(&z2zE2745, zRlpDecode);
        COPY(zexception)(current_exception, z2zE2745);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/lib/rlp/tx.sail:146.12-146.41");
        KILL(zexception)(&z2zE2745);
        goto end_block_exception_3043;
        /* unreachable after throw */
        KILL(zexception)(&z2zE2745);
      } else {  z3zE2619 = UNIT;  }
      bool z2zE2746;
      z2zE2746 = (zcount < UINT64_C(9));
      if (z2zE2746) {
        uint64_t z2zE2747;
        {    z2zE2747 = (zcount + UINT64_C(1));
        }
        {
          z3zE2615 = zdecode_blob_hash_items(znext, z2zE2747);
          if (have_exception) {  goto end_block_exception_3043;  }
        }
      } else {
        struct zexception z2zE2748;
        CREATE(zexception)(&z2zE2748);
        zInvalidBlock(&z2zE2748, zRlpDecode);
        COPY(zexception)(current_exception, z2zE2748);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/lib/rlp/tx.sail:151.12-151.41");
        KILL(zexception)(&z2zE2748);
        goto end_block_exception_3043;
        /* unreachable after throw */
        KILL(zexception)(&z2zE2748);
      }
      goto finish_match_3040;
    }
  case_3041: ;
    sail_match_failure("decode_blob_hash_items");
  finish_match_3040: ;
    z8zE189 = z3zE2615;
  }
end_function_3042: ;
  return z8zE189;
end_block_exception_3043: ;

  return UINT64_C(0xdeadc0de);
}

struct zBlobHashes zdecode_blob_hashes(struct zRlpFieldRef zf)
{
  struct zBlobHashes z8zE190;
  struct zByteSliceFields zbytes;
  zbytes = zrlp_ref_content(zf);
  uint64_t z2zE2732;
  {
    struct zByteSliceFields z2zE2731;
    {
      z2zE2731 = zrlp_ref_cursor(zf);
      if (have_exception) {  goto end_block_exception_3039;  }
    }
    {
      z2zE2732 = zdecode_blob_hash_items(z2zE2731, UINT64_C(0));
      if (have_exception) {  goto end_block_exception_3039;  }
    }
  }
  struct zBlobHashes z3zE2614;
  z3zE2614.zbytes = zbytes;
  z3zE2614.zcount = z2zE2732;
  z8zE190 = z3zE2614;
end_function_3038: ;
  return z8zE190;
end_block_exception_3039: ;
  struct zByteSliceFields z8zE797 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zBlobHashes z8zE796 = { .zbytes = z8zE797, .zcount = UINT64_C(0xdeadc0de) };
  return z8zE796;
}

void create_letbind_117(void) {    CREATE(zAuthorizzationDecode)(&zEMPTY_AUTHORIZATION_DECODE);

  struct zAuthorizzationDecode z3zE133;
  CREATE(zAuthorizzationDecode)(&z3zE133);
  struct zAuthorizzationDecode z3zE131;
  CREATE(zAuthorizzationDecode)(&z3zE131);
  zz5listz8z5structz0zzAuthorizzzzationz9 z3zE132;
  CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&z3zE132);
  COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&((&z3zE131)->zauthorizzations), z3zE132);
  KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z3zE132);
  z3zE131.zcount = UINT64_C(0);
  COPY(zAuthorizzationDecode)(&z3zE133, z3zE131);
  KILL(zAuthorizzationDecode)(&z3zE131);
  COPY(zAuthorizzationDecode)(&zEMPTY_AUTHORIZATION_DECODE, z3zE133);
  KILL(zAuthorizzationDecode)(&z3zE133);
let_end_321: ;
}
void kill_letbind_117(void) {    KILL(zAuthorizzationDecode)(&zEMPTY_AUTHORIZATION_DECODE);
}

void zdecode_auth_tuples(struct zAuthorizzationDecode *z8zE191, struct zByteSliceFields zcursor)
{
  bool z2zE2709;
  {
    uint64_t z2zE2708;
    z2zE2708 = zcursor.zlen;
    z2zE2709 = (z2zE2708 == UINT64_C(0));
  }
  if (z2zE2709) {
    struct zAuthorizzationDecode z3zE2612;
    CREATE(zAuthorizzationDecode)(&z3zE2612);
    zz5listz8z5structz0zzAuthorizzzzationz9 z3zE2613;
    CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&z3zE2613);
    COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&((&z3zE2612)->zauthorizzations), z3zE2613);
    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z3zE2613);
    z3zE2612.zcount = UINT64_C(0);
    COPY(zAuthorizzationDecode)((*(&z8zE191)), z3zE2612);
    KILL(zAuthorizzationDecode)(&z3zE2612);
  } else {
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2710;
    {
      z2zE2710 = zrlp_cursor_pop(zcursor);
      if (have_exception) {  goto end_block_exception_3037;  }
    }
    struct zAuthorizzationDecode z3zE2586;
    CREATE(zAuthorizzationDecode)(&z3zE2586);
    {
      struct zRlpFieldRef ztuple;
      ztuple = z2zE2710.ztup0;
      struct zByteSliceFields znext;
      znext = z2zE2710.ztup1;
      struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2712;
      {
        struct zByteSliceFields z2zE2711;
        {
          z2zE2711 = zrlp_ref_cursor(ztuple);
          if (have_exception) {
            KILL(zAuthorizzationDecode)(&z3zE2586);
            goto end_block_exception_3037;
          }
        }
        {
          z2zE2712 = zrlp_cursor_pop(z2zE2711);
          if (have_exception) {
            KILL(zAuthorizzationDecode)(&z3zE2586);
            goto end_block_exception_3037;
          }
        }
      }
      struct zAuthorizzationDecode z3zE2587;
      CREATE(zAuthorizzationDecode)(&z3zE2587);
      {
        struct zRlpFieldRef zchain_f;
        zchain_f = z2zE2712.ztup0;
        struct zByteSliceFields zfields;
        zfields = z2zE2712.ztup1;
        struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2713;
        {
          z2zE2713 = zrlp_cursor_pop(zfields);
          if (have_exception) {
            KILL(zAuthorizzationDecode)(&z3zE2586);
            KILL(zAuthorizzationDecode)(&z3zE2587);
            goto end_block_exception_3037;
          }
        }
        struct zAuthorizzationDecode z3zE2588;
        CREATE(zAuthorizzationDecode)(&z3zE2588);
        {
          struct zRlpFieldRef zaddr_f;
          zaddr_f = z2zE2713.ztup0;
          struct zByteSliceFields z1zE84;
          z1zE84 = z2zE2713.ztup1;
          struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2714;
          {
            z2zE2714 = zrlp_cursor_pop(z1zE84);
            if (have_exception) {
              KILL(zAuthorizzationDecode)(&z3zE2586);
              KILL(zAuthorizzationDecode)(&z3zE2587);
              KILL(zAuthorizzationDecode)(&z3zE2588);
              goto end_block_exception_3037;
            }
          }
          struct zAuthorizzationDecode z3zE2589;
          CREATE(zAuthorizzationDecode)(&z3zE2589);
          {
            struct zRlpFieldRef znonce_f;
            znonce_f = z2zE2714.ztup0;
            struct zByteSliceFields z1zE85;
            z1zE85 = z2zE2714.ztup1;
            struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2715;
            {
              z2zE2715 = zrlp_cursor_pop(z1zE85);
              if (have_exception) {
                KILL(zAuthorizzationDecode)(&z3zE2586);
                KILL(zAuthorizzationDecode)(&z3zE2587);
                KILL(zAuthorizzationDecode)(&z3zE2588);
                KILL(zAuthorizzationDecode)(&z3zE2589);
                goto end_block_exception_3037;
              }
            }
            struct zAuthorizzationDecode z3zE2590;
            CREATE(zAuthorizzationDecode)(&z3zE2590);
            {
              struct zRlpFieldRef zy_f;
              zy_f = z2zE2715.ztup0;
              struct zByteSliceFields z1zE86;
              z1zE86 = z2zE2715.ztup1;
              struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2716;
              {
                z2zE2716 = zrlp_cursor_pop(z1zE86);
                if (have_exception) {
                  KILL(zAuthorizzationDecode)(&z3zE2586);
                  KILL(zAuthorizzationDecode)(&z3zE2587);
                  KILL(zAuthorizzationDecode)(&z3zE2588);
                  KILL(zAuthorizzationDecode)(&z3zE2589);
                  KILL(zAuthorizzationDecode)(&z3zE2590);
                  goto end_block_exception_3037;
                }
              }
              struct zAuthorizzationDecode z3zE2591;
              CREATE(zAuthorizzationDecode)(&z3zE2591);
              {
                struct zRlpFieldRef zr_f;
                zr_f = z2zE2716.ztup0;
                struct zByteSliceFields z1zE87;
                z1zE87 = z2zE2716.ztup1;
                struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2717;
                {
                  z2zE2717 = zrlp_cursor_pop(z1zE87);
                  if (have_exception) {
                    KILL(zAuthorizzationDecode)(&z3zE2586);
                    KILL(zAuthorizzationDecode)(&z3zE2587);
                    KILL(zAuthorizzationDecode)(&z3zE2588);
                    KILL(zAuthorizzationDecode)(&z3zE2589);
                    KILL(zAuthorizzationDecode)(&z3zE2590);
                    KILL(zAuthorizzationDecode)(&z3zE2591);
                    goto end_block_exception_3037;
                  }
                }
                struct zAuthorizzationDecode z3zE2592;
                CREATE(zAuthorizzationDecode)(&z3zE2592);
                {
                  struct zRlpFieldRef zs_f;
                  zs_f = z2zE2717.ztup0;
                  struct zByteSliceFields z1zE88;
                  z1zE88 = z2zE2717.ztup1;
                  unit z3zE2593;
                  {
                    z3zE2593 = zrlp_cursor_expect_end(z1zE88);
                    if (have_exception) {
                      KILL(zAuthorizzationDecode)(&z3zE2586);
                      KILL(zAuthorizzationDecode)(&z3zE2587);
                      KILL(zAuthorizzationDecode)(&z3zE2588);
                      KILL(zAuthorizzationDecode)(&z3zE2589);
                      KILL(zAuthorizzationDecode)(&z3zE2590);
                      KILL(zAuthorizzationDecode)(&z3zE2591);
                      KILL(zAuthorizzationDecode)(&z3zE2592);
                      goto end_block_exception_3037;
                    }
                  }
                  sail_u256 zchain_id;
                  {
                    zchain_id = zrlp_ref_uint_word(zchain_f);
                    if (have_exception) {
                      KILL(zAuthorizzationDecode)(&z3zE2586);
                      KILL(zAuthorizzationDecode)(&z3zE2587);
                      KILL(zAuthorizzationDecode)(&z3zE2588);
                      KILL(zAuthorizzationDecode)(&z3zE2589);
                      KILL(zAuthorizzationDecode)(&z3zE2590);
                      KILL(zAuthorizzationDecode)(&z3zE2591);
                      KILL(zAuthorizzationDecode)(&z3zE2592);
                      goto end_block_exception_3037;
                    }
                  }
                  uint64_t zauth_nonce;
                  {
                    zauth_nonce = zrlp_ref_uint64(znonce_f);
                    if (have_exception) {
                      KILL(zAuthorizzationDecode)(&z3zE2586);
                      KILL(zAuthorizzationDecode)(&z3zE2587);
                      KILL(zAuthorizzationDecode)(&z3zE2588);
                      KILL(zAuthorizzationDecode)(&z3zE2589);
                      KILL(zAuthorizzationDecode)(&z3zE2590);
                      KILL(zAuthorizzationDecode)(&z3zE2591);
                      KILL(zAuthorizzationDecode)(&z3zE2592);
                      goto end_block_exception_3037;
                    }
                  }
                  uint64_t zy_value;
                  {
                    zy_value = zrlp_ref_uint64(zy_f);
                    if (have_exception) {
                      KILL(zAuthorizzationDecode)(&z3zE2586);
                      KILL(zAuthorizzationDecode)(&z3zE2587);
                      KILL(zAuthorizzationDecode)(&z3zE2588);
                      KILL(zAuthorizzationDecode)(&z3zE2589);
                      KILL(zAuthorizzationDecode)(&z3zE2590);
                      KILL(zAuthorizzationDecode)(&z3zE2591);
                      KILL(zAuthorizzationDecode)(&z3zE2592);
                      goto end_block_exception_3037;
                    }
                  }
                  bool zy_valid;
                  zy_valid = (!(UINT64_C(1) < zy_value));
                  uint64_t zy;
                  {
                    bool z2zE2730;
                    z2zE2730 = (zy_value == UINT64_C(0));
                    if (z2zE2730) {  zy = UINT64_C(0);  } else {  zy = UINT64_C(1);  }
                  }
                  sail_u256 zr;
                  {
                    zr = zrlp_ref_uint_word(zr_f);
                    if (have_exception) {
                      KILL(zAuthorizzationDecode)(&z3zE2586);
                      KILL(zAuthorizzationDecode)(&z3zE2587);
                      KILL(zAuthorizzationDecode)(&z3zE2588);
                      KILL(zAuthorizzationDecode)(&z3zE2589);
                      KILL(zAuthorizzationDecode)(&z3zE2590);
                      KILL(zAuthorizzationDecode)(&z3zE2591);
                      KILL(zAuthorizzationDecode)(&z3zE2592);
                      goto end_block_exception_3037;
                    }
                  }
                  sail_u256 zs;
                  {
                    zs = zrlp_ref_uint_word(zs_f);
                    if (have_exception) {
                      KILL(zAuthorizzationDecode)(&z3zE2586);
                      KILL(zAuthorizzationDecode)(&z3zE2587);
                      KILL(zAuthorizzationDecode)(&z3zE2588);
                      KILL(zAuthorizzationDecode)(&z3zE2589);
                      KILL(zAuthorizzationDecode)(&z3zE2590);
                      KILL(zAuthorizzationDecode)(&z3zE2591);
                      KILL(zAuthorizzationDecode)(&z3zE2592);
                      goto end_block_exception_3037;
                    }
                  }
                  sail_fixed_bytes_20 zauth_addr;
                  {
                    sail_u256 z2zE2729;
                    {
                      z2zE2729 = zrlp_ref_word(zaddr_f);
                      if (have_exception) {
                        KILL(zAuthorizzationDecode)(&z3zE2586);
                        KILL(zAuthorizzationDecode)(&z3zE2587);
                        KILL(zAuthorizzationDecode)(&z3zE2588);
                        KILL(zAuthorizzationDecode)(&z3zE2589);
                        KILL(zAuthorizzationDecode)(&z3zE2590);
                        KILL(zAuthorizzationDecode)(&z3zE2591);
                        KILL(zAuthorizzationDecode)(&z3zE2592);
                        goto end_block_exception_3037;
                      }
                    }
                    zauth_addr = evmsail_word_to_address(z2zE2729);
                  }
                  struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE2719;
                  if (zy_valid) {
                    sail_fixed_bytes_32 z2zE2718;
                    z2zE2718 = zauth_signing_hash(zchain_id, zauth_addr, zauth_nonce);
                    z2zE2719 = zecrecover_addr(z2zE2718, zy, zr, zs);
                  } else {
                    struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z3zE2594;
                    z3zE2594.ztup0 = false;
                    z3zE2594.ztup1 = zZERO_ADDRESS;
                    z2zE2719 = z3zE2594;
                  }
                  struct zAuthorizzationDecode z3zE2595;
                  CREATE(zAuthorizzationDecode)(&z3zE2595);
                  {
                    bool zok;
                    zok = z2zE2719.ztup0;
                    sail_fixed_bytes_20 zauthority;
                    zauthority = z2zE2719.ztup1;
                    bool znonce_valid;
                    znonce_valid = (zauth_nonce != UINT64_C(18446744073709551615));
                    struct zAuthorizzation zauthorizzation;
                    {
                      bool z2zE2728;
                      {
                        bool z3zE2601;
                        if (zok) {
                          bool z3zE2600;
                          if (zy_valid) {
                            bool z2zE2727;
                            z2zE2727 = zword_ult(zZERO_WORD, zr);
                            bool z3zE2599;
                            if (z2zE2727) {
                              bool z2zE2726;
                              z2zE2726 = zword_ult(zr, zSECP_N_FULL);
                              bool z3zE2598;
                              if (z2zE2726) {
                                bool z2zE2725;
                                z2zE2725 = zword_ult(zZERO_WORD, zs);
                                bool z3zE2597;
                                if (z2zE2725) {
                                  bool z2zE2724;
                                  z2zE2724 = zword_ule(zs, zSECP_N_HALF);
                                  bool z3zE2596;
                                  if (z2zE2724) {  z3zE2596 = znonce_valid;  } else {  z3zE2596 = false;  }
                                  z3zE2597 = z3zE2596;
                                } else {  z3zE2597 = false;  }
                                z3zE2598 = z3zE2597;
                              } else {  z3zE2598 = false;  }
                              z3zE2599 = z3zE2598;
                            } else {  z3zE2599 = false;  }
                            z3zE2600 = z3zE2599;
                          } else {  z3zE2600 = false;  }
                          z3zE2601 = z3zE2600;
                        } else {  z3zE2601 = false;  }
                        z2zE2728 = z3zE2601;
                      }
                      struct zAuthorizzation z3zE2602;
                      z3zE2602.zaddress = zauth_addr;
                      z3zE2602.zauthority = zauthority;
                      z3zE2602.zchain_id = zchain_id;
                      z3zE2602.znonce = zauth_nonce;
                      z3zE2602.zvalid_sig = z2zE2728;
                      zauthorizzation = z3zE2602;
                    }
                    struct zAuthorizzationDecode ztail;
                    CREATE(zAuthorizzationDecode)(&ztail);
                    {
                      zdecode_auth_tuples(&ztail, znext);
                      if (have_exception) {
                        KILL(zAuthorizzationDecode)(&z3zE2586);
                        KILL(zAuthorizzationDecode)(&z3zE2587);
                        KILL(zAuthorizzationDecode)(&z3zE2588);
                        KILL(zAuthorizzationDecode)(&z3zE2589);
                        KILL(zAuthorizzationDecode)(&z3zE2590);
                        KILL(zAuthorizzationDecode)(&z3zE2591);
                        KILL(zAuthorizzationDecode)(&z3zE2592);
                        KILL(zAuthorizzationDecode)(&z3zE2595);
                        KILL(zAuthorizzationDecode)(&ztail);
                        goto end_block_exception_3037;
                      }
                    }
                    zz5listz8z5structz0zzAuthorizzzzationz9 z2zE2721;
                    CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE2721);
                    {
                      zz5listz8z5structz0zzAuthorizzzzationz9 z2zE2720;
                      CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE2720);
                      COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE2720, ztail.zauthorizzations);
                      zconsz3z5structz0zzAuthorizzzzation(&z2zE2721, zauthorizzation, z2zE2720);
                      KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE2720);
                    }
                    uint64_t z2zE2723;
                    {
                      uint64_t z2zE2722;
                      z2zE2722 = ztail.zcount;
                      z2zE2723 = zrlp_item_count_incrementzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2722);
                    }
                    struct zAuthorizzationDecode z3zE2603;
                    CREATE(zAuthorizzationDecode)(&z3zE2603);
                    COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&((&z3zE2603)->zauthorizzations), z2zE2721);
                    z3zE2603.zcount = z2zE2723;
                    COPY(zAuthorizzationDecode)(&z3zE2595, z3zE2603);
                    KILL(zAuthorizzationDecode)(&z3zE2603);
                    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE2721);
                    KILL(zAuthorizzationDecode)(&ztail);
                    goto finish_match_3034;
                  }
                case_3035: ;
                  sail_match_failure("decode_auth_tuples");
                finish_match_3034: ;
                  COPY(zAuthorizzationDecode)(&z3zE2592, z3zE2595);
                  KILL(zAuthorizzationDecode)(&z3zE2595);
                  goto finish_match_3032;
                }
              case_3033: ;
                sail_match_failure("decode_auth_tuples");
              finish_match_3032: ;
                COPY(zAuthorizzationDecode)(&z3zE2591, z3zE2592);
                KILL(zAuthorizzationDecode)(&z3zE2592);
                goto finish_match_3030;
              }
            case_3031: ;
              sail_match_failure("decode_auth_tuples");
            finish_match_3030: ;
              COPY(zAuthorizzationDecode)(&z3zE2590, z3zE2591);
              KILL(zAuthorizzationDecode)(&z3zE2591);
              goto finish_match_3028;
            }
          case_3029: ;
            sail_match_failure("decode_auth_tuples");
          finish_match_3028: ;
            COPY(zAuthorizzationDecode)(&z3zE2589, z3zE2590);
            KILL(zAuthorizzationDecode)(&z3zE2590);
            goto finish_match_3026;
          }
        case_3027: ;
          sail_match_failure("decode_auth_tuples");
        finish_match_3026: ;
          COPY(zAuthorizzationDecode)(&z3zE2588, z3zE2589);
          KILL(zAuthorizzationDecode)(&z3zE2589);
          goto finish_match_3024;
        }
      case_3025: ;
        sail_match_failure("decode_auth_tuples");
      finish_match_3024: ;
        COPY(zAuthorizzationDecode)(&z3zE2587, z3zE2588);
        KILL(zAuthorizzationDecode)(&z3zE2588);
        goto finish_match_3022;
      }
    case_3023: ;
      sail_match_failure("decode_auth_tuples");
    finish_match_3022: ;
      COPY(zAuthorizzationDecode)(&z3zE2586, z3zE2587);
      KILL(zAuthorizzationDecode)(&z3zE2587);
      goto finish_match_3020;
    }
  case_3021: ;
    sail_match_failure("decode_auth_tuples");
  finish_match_3020: ;
    COPY(zAuthorizzationDecode)((*(&z8zE191)), z3zE2586);
    KILL(zAuthorizzationDecode)(&z3zE2586);
  }
end_function_3036: ;
  goto end_function_3649;
end_block_exception_3037: ;
  goto end_function_3649;
end_function_3649: ;
}

void zdecode_auth_list(struct ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9 *z8zE192, struct zRlpFieldRef zf)
{
  bool z2zE2703;
  {
    uint64_t z2zE2702;
    {
      struct zByteSliceFields z2zE2701;
      z2zE2701 = zf.zsource;
      z2zE2702 = z2zE2701.zlen;
    }
    z2zE2703 = (!(UINT64_C(1073741824) < z2zE2702));
  }
  if (z2zE2703) {
    struct zAuthorizzationDecode zdecoded;
    CREATE(zAuthorizzationDecode)(&zdecoded);
    {
      struct zByteSliceFields z2zE2706;
      {
        z2zE2706 = zrlp_ref_cursor(zf);
        if (have_exception) {
          KILL(zAuthorizzationDecode)(&zdecoded);
          goto end_block_exception_3019;
        }
      }
      {
        zdecode_auth_tuples(&zdecoded, z2zE2706);
        if (have_exception) {
          KILL(zAuthorizzationDecode)(&zdecoded);
          goto end_block_exception_3019;
        }
      }
    }
    zz5listz8z5structz0zzAuthorizzzzationz9 z2zE2704;
    CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE2704);
    COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE2704, zdecoded.zauthorizzations);
    uint64_t z2zE2705;
    z2zE2705 = zdecoded.zcount;
    struct ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9 z3zE2585;
    CREATE(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z3zE2585);
    COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&((&z3zE2585)->ztup0), z2zE2704);
    z3zE2585.ztup1 = z2zE2705;
    COPY(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)((*(&z8zE192)), z3zE2585);
    KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z3zE2585);
    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE2704);
    KILL(zAuthorizzationDecode)(&zdecoded);
  } else {
    struct zexception z2zE2707;
    CREATE(zexception)(&z2zE2707);
    zInvalidBlock(&z2zE2707, zRlpDecode);
    COPY(zexception)(current_exception, z2zE2707);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/tx.sail:260.8-260.37");
    KILL(zexception)(&z2zE2707);
    goto end_block_exception_3019;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2707);
  }
end_function_3018: ;
  goto end_function_3648;
end_block_exception_3019: ;
  goto end_function_3648;
end_function_3648: ;
}

struct zByteSliceFields ztx_input_span(struct zRlpFieldRef zdata)
{
  struct zByteSliceFields z8zE193;
  struct zByteSliceFields zcontent;
  zcontent = zrlp_ref_content(zdata);
  bool z2zE2699;
  {
    uint64_t z2zE2698;
    z2zE2698 = zcontent.zlen;
    z2zE2699 = (!(UINT64_C(1073741824) < z2zE2698));
  }
  if (z2zE2699) {  z8zE193 = zcontent;  } else {
    struct zexception z2zE2700;
    CREATE(zexception)(&z2zE2700);
    zInvalidBlock(&z2zE2700, zRlpDecode);
    COPY(zexception)(current_exception, z2zE2700);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/tx.sail:276.8-276.37");
    KILL(zexception)(&z2zE2700);
    goto end_block_exception_3017;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2700);
  }
end_function_3016: ;
  return z8zE193;
end_block_exception_3017: ;
  struct zByteSliceFields z8zE798 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE798;
}

struct zByteSliceFields ztx_sig_span(struct zRlpFieldRef zfirst, struct zRlpFieldRef zsignature)
{
  struct zByteSliceFields z8zE194;
  uint64_t zstart;
  {
    struct zByteSliceFields z2zE2697;
    z2zE2697 = zfirst.zsource;
    zstart = z2zE2697.zoff;
  }
  uint64_t zstop;
  {
    struct zByteSliceFields z2zE2696;
    z2zE2696 = zsignature.zsource;
    zstop = z2zE2696.zoff;
  }
  uint64_t zstart_offset;
  zstart_offset = zstart;
  uint64_t zstop_offset;
  zstop_offset = zstop;
  bool z2zE2691;
  z2zE2691 = (zstop_offset < zstart_offset);
  enum zByteSource z2zE2694;
  {
    struct zByteSliceFields z2zE2693;
    z2zE2693 = zfirst.zsource;
    z2zE2694 = z2zE2693.zsource;
  }
  uint64_t z2zE2695;
  {    z2zE2695 = (zstop_offset - zstart_offset);
  }
  z8zE194 = zbyte_slice(z2zE2694, zstart_offset, z2zE2695);
end_function_3014: ;
  return z8zE194;
end_block_exception_3015: ;
  struct zByteSliceFields z8zE799 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE799;
}

uint64_t zrlp_ref_gas(struct zRlpFieldRef zf, enum zFork zfork)
{
  uint64_t z8zE195;
  uint64_t zvalue;
  {
    zvalue = zrlp_ref_uint64(zf);
    if (have_exception) {  goto end_block_exception_3013;  }
  }
  bool z2zE2689;
  {
    bool z2zE2688;
    z2zE2688 = zfork_gteq(zfork, zOsaka);
    bool z3zE2584;
    if (z2zE2688) {
      bool z2zE2687;
      z2zE2687 = zfork_lt(zfork, zAmsterdam);
      bool z3zE2583;
      if (z2zE2687) {  z3zE2583 = (zOSAKA_TRANSACTION_GAS_LIMIT_VALUE < zvalue);  } else {  z3zE2583 = false;  }
      z3zE2584 = z3zE2583;
    } else {  z3zE2584 = false;  }
    z2zE2689 = z3zE2584;
  }
  if (z2zE2689) {
    struct zexception z2zE2690;
    CREATE(zexception)(&z2zE2690);
    zInvalidBlock(&z2zE2690, zGasUsedExceedsLimit);
    COPY(zexception)(current_exception, z2zE2690);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/splices/c_optimized.sail:450.8-450.47");
    KILL(zexception)(&z2zE2690);
    goto end_block_exception_3013;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2690);
  } else {  z8zE195 = zvalue;  }
end_function_3012: ;
  return z8zE195;
end_block_exception_3013: ;

  return UINT64_C(0xdeadc0de);
}

void zdecode_legacy_tx(struct zTransaction *z8zE196, struct zByteSliceFields ztx, struct zByteSliceFields zpubkey, enum zFork zfork, sail_fixed_bytes_20 zsender, struct zByteSliceFields zfields)
{
  struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2666;
  {
    z2zE2666 = zrlp_cursor_pop(zfields);
    if (have_exception) {  goto end_block_exception_3011;  }
  }
  struct zTransaction z3zE2560;
  CREATE(zTransaction)(&z3zE2560);
  {
    struct zRlpFieldRef znonce_f;
    znonce_f = z2zE2666.ztup0;
    struct zByteSliceFields z1zE75;
    z1zE75 = z2zE2666.ztup1;
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2667;
    {
      z2zE2667 = zrlp_cursor_pop(z1zE75);
      if (have_exception) {
        KILL(zTransaction)(&z3zE2560);
        goto end_block_exception_3011;
      }
    }
    struct zTransaction z3zE2561;
    CREATE(zTransaction)(&z3zE2561);
    {
      struct zRlpFieldRef zgp_f;
      zgp_f = z2zE2667.ztup0;
      struct zByteSliceFields z1zE76;
      z1zE76 = z2zE2667.ztup1;
      struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2668;
      {
        z2zE2668 = zrlp_cursor_pop(z1zE76);
        if (have_exception) {
          KILL(zTransaction)(&z3zE2560);
          KILL(zTransaction)(&z3zE2561);
          goto end_block_exception_3011;
        }
      }
      struct zTransaction z3zE2562;
      CREATE(zTransaction)(&z3zE2562);
      {
        struct zRlpFieldRef zgas_f;
        zgas_f = z2zE2668.ztup0;
        struct zByteSliceFields z1zE77;
        z1zE77 = z2zE2668.ztup1;
        struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2669;
        {
          z2zE2669 = zrlp_cursor_pop(z1zE77);
          if (have_exception) {
            KILL(zTransaction)(&z3zE2560);
            KILL(zTransaction)(&z3zE2561);
            KILL(zTransaction)(&z3zE2562);
            goto end_block_exception_3011;
          }
        }
        struct zTransaction z3zE2563;
        CREATE(zTransaction)(&z3zE2563);
        {
          struct zRlpFieldRef zto_f;
          zto_f = z2zE2669.ztup0;
          struct zByteSliceFields z1zE78;
          z1zE78 = z2zE2669.ztup1;
          struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2670;
          {
            z2zE2670 = zrlp_cursor_pop(z1zE78);
            if (have_exception) {
              KILL(zTransaction)(&z3zE2560);
              KILL(zTransaction)(&z3zE2561);
              KILL(zTransaction)(&z3zE2562);
              KILL(zTransaction)(&z3zE2563);
              goto end_block_exception_3011;
            }
          }
          struct zTransaction z3zE2564;
          CREATE(zTransaction)(&z3zE2564);
          {
            struct zRlpFieldRef zvalue_f;
            zvalue_f = z2zE2670.ztup0;
            struct zByteSliceFields z1zE79;
            z1zE79 = z2zE2670.ztup1;
            struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2671;
            {
              z2zE2671 = zrlp_cursor_pop(z1zE79);
              if (have_exception) {
                KILL(zTransaction)(&z3zE2560);
                KILL(zTransaction)(&z3zE2561);
                KILL(zTransaction)(&z3zE2562);
                KILL(zTransaction)(&z3zE2563);
                KILL(zTransaction)(&z3zE2564);
                goto end_block_exception_3011;
              }
            }
            struct zTransaction z3zE2565;
            CREATE(zTransaction)(&z3zE2565);
            {
              struct zRlpFieldRef zdata_f;
              zdata_f = z2zE2671.ztup0;
              struct zByteSliceFields z1zE80;
              z1zE80 = z2zE2671.ztup1;
              struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2672;
              {
                z2zE2672 = zrlp_cursor_pop(z1zE80);
                if (have_exception) {
                  KILL(zTransaction)(&z3zE2560);
                  KILL(zTransaction)(&z3zE2561);
                  KILL(zTransaction)(&z3zE2562);
                  KILL(zTransaction)(&z3zE2563);
                  KILL(zTransaction)(&z3zE2564);
                  KILL(zTransaction)(&z3zE2565);
                  goto end_block_exception_3011;
                }
              }
              struct zTransaction z3zE2566;
              CREATE(zTransaction)(&z3zE2566);
              {
                struct zRlpFieldRef zv_f;
                zv_f = z2zE2672.ztup0;
                struct zByteSliceFields z1zE81;
                z1zE81 = z2zE2672.ztup1;
                struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2673;
                {
                  z2zE2673 = zrlp_cursor_pop(z1zE81);
                  if (have_exception) {
                    KILL(zTransaction)(&z3zE2560);
                    KILL(zTransaction)(&z3zE2561);
                    KILL(zTransaction)(&z3zE2562);
                    KILL(zTransaction)(&z3zE2563);
                    KILL(zTransaction)(&z3zE2564);
                    KILL(zTransaction)(&z3zE2565);
                    KILL(zTransaction)(&z3zE2566);
                    goto end_block_exception_3011;
                  }
                }
                struct zTransaction z3zE2567;
                CREATE(zTransaction)(&z3zE2567);
                {
                  struct zRlpFieldRef zr_f;
                  zr_f = z2zE2673.ztup0;
                  struct zByteSliceFields z1zE82;
                  z1zE82 = z2zE2673.ztup1;
                  struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2674;
                  {
                    z2zE2674 = zrlp_cursor_pop(z1zE82);
                    if (have_exception) {
                      KILL(zTransaction)(&z3zE2560);
                      KILL(zTransaction)(&z3zE2561);
                      KILL(zTransaction)(&z3zE2562);
                      KILL(zTransaction)(&z3zE2563);
                      KILL(zTransaction)(&z3zE2564);
                      KILL(zTransaction)(&z3zE2565);
                      KILL(zTransaction)(&z3zE2566);
                      KILL(zTransaction)(&z3zE2567);
                      goto end_block_exception_3011;
                    }
                  }
                  struct zTransaction z3zE2568;
                  CREATE(zTransaction)(&z3zE2568);
                  {
                    struct zRlpFieldRef zs_f;
                    zs_f = z2zE2674.ztup0;
                    struct zByteSliceFields z1zE83;
                    z1zE83 = z2zE2674.ztup1;
                    unit z3zE2569;
                    {
                      z3zE2569 = zrlp_cursor_expect_end(z1zE83);
                      if (have_exception) {
                        KILL(zTransaction)(&z3zE2560);
                        KILL(zTransaction)(&z3zE2561);
                        KILL(zTransaction)(&z3zE2562);
                        KILL(zTransaction)(&z3zE2563);
                        KILL(zTransaction)(&z3zE2564);
                        KILL(zTransaction)(&z3zE2565);
                        KILL(zTransaction)(&z3zE2566);
                        KILL(zTransaction)(&z3zE2567);
                        KILL(zTransaction)(&z3zE2568);
                        goto end_block_exception_3011;
                      }
                    }
                    sail_u256 zv;
                    {
                      zv = zrlp_ref_word(zv_f);
                      if (have_exception) {
                        KILL(zTransaction)(&z3zE2560);
                        KILL(zTransaction)(&z3zE2561);
                        KILL(zTransaction)(&z3zE2562);
                        KILL(zTransaction)(&z3zE2563);
                        KILL(zTransaction)(&z3zE2564);
                        KILL(zTransaction)(&z3zE2565);
                        KILL(zTransaction)(&z3zE2566);
                        KILL(zTransaction)(&z3zE2567);
                        KILL(zTransaction)(&z3zE2568);
                        goto end_block_exception_3011;
                      }
                    }
                    sail_u256 zgp;
                    {
                      zgp = zrlp_ref_uint_word(zgp_f);
                      if (have_exception) {
                        KILL(zTransaction)(&z3zE2560);
                        KILL(zTransaction)(&z3zE2561);
                        KILL(zTransaction)(&z3zE2562);
                        KILL(zTransaction)(&z3zE2563);
                        KILL(zTransaction)(&z3zE2564);
                        KILL(zTransaction)(&z3zE2565);
                        KILL(zTransaction)(&z3zE2566);
                        KILL(zTransaction)(&z3zE2567);
                        KILL(zTransaction)(&z3zE2568);
                        goto end_block_exception_3011;
                      }
                    }
                    sail_u256 z2zE2675;
                    {
                      z2zE2675 = zrlp_ref_uint_word(znonce_f);
                      if (have_exception) {
                        KILL(zTransaction)(&z3zE2560);
                        KILL(zTransaction)(&z3zE2561);
                        KILL(zTransaction)(&z3zE2562);
                        KILL(zTransaction)(&z3zE2563);
                        KILL(zTransaction)(&z3zE2564);
                        KILL(zTransaction)(&z3zE2565);
                        KILL(zTransaction)(&z3zE2566);
                        KILL(zTransaction)(&z3zE2567);
                        KILL(zTransaction)(&z3zE2568);
                        goto end_block_exception_3011;
                      }
                    }
                    uint64_t z2zE2676;
                    {
                      z2zE2676 = zrlp_ref_gas(zgas_f, zfork);
                      if (have_exception) {
                        KILL(zTransaction)(&z3zE2560);
                        KILL(zTransaction)(&z3zE2561);
                        KILL(zTransaction)(&z3zE2562);
                        KILL(zTransaction)(&z3zE2563);
                        KILL(zTransaction)(&z3zE2564);
                        KILL(zTransaction)(&z3zE2565);
                        KILL(zTransaction)(&z3zE2566);
                        KILL(zTransaction)(&z3zE2567);
                        KILL(zTransaction)(&z3zE2568);
                        goto end_block_exception_3011;
                      }
                    }
                    bool z2zE2678;
                    {
                      uint64_t z2zE2677;
                      z2zE2677 = zto_f.zcontent_len;
                      z2zE2678 = (z2zE2677 == UINT64_C(0));
                    }
                    sail_fixed_bytes_20 z2zE2680;
                    {
                      sail_u256 z2zE2679;
                      {
                        z2zE2679 = zrlp_ref_word(zto_f);
                        if (have_exception) {
                          KILL(zTransaction)(&z3zE2560);
                          KILL(zTransaction)(&z3zE2561);
                          KILL(zTransaction)(&z3zE2562);
                          KILL(zTransaction)(&z3zE2563);
                          KILL(zTransaction)(&z3zE2564);
                          KILL(zTransaction)(&z3zE2565);
                          KILL(zTransaction)(&z3zE2566);
                          KILL(zTransaction)(&z3zE2567);
                          KILL(zTransaction)(&z3zE2568);
                          goto end_block_exception_3011;
                        }
                      }
                      z2zE2680 = evmsail_word_to_address(z2zE2679);
                    }
                    sail_u256 z2zE2681;
                    {
                      z2zE2681 = zrlp_ref_uint_word(zvalue_f);
                      if (have_exception) {
                        KILL(zTransaction)(&z3zE2560);
                        KILL(zTransaction)(&z3zE2561);
                        KILL(zTransaction)(&z3zE2562);
                        KILL(zTransaction)(&z3zE2563);
                        KILL(zTransaction)(&z3zE2564);
                        KILL(zTransaction)(&z3zE2565);
                        KILL(zTransaction)(&z3zE2566);
                        KILL(zTransaction)(&z3zE2567);
                        KILL(zTransaction)(&z3zE2568);
                        goto end_block_exception_3011;
                      }
                    }
                    struct zByteSliceFields z2zE2682;
                    {
                      z2zE2682 = ztx_input_span(zdata_f);
                      if (have_exception) {
                        KILL(zTransaction)(&z3zE2560);
                        KILL(zTransaction)(&z3zE2561);
                        KILL(zTransaction)(&z3zE2562);
                        KILL(zTransaction)(&z3zE2563);
                        KILL(zTransaction)(&z3zE2564);
                        KILL(zTransaction)(&z3zE2565);
                        KILL(zTransaction)(&z3zE2566);
                        KILL(zTransaction)(&z3zE2567);
                        KILL(zTransaction)(&z3zE2568);
                        goto end_block_exception_3011;
                      }
                    }
                    sail_fixed_bytes_32 z2zE2684;
                    {
                      struct zByteSliceFields z2zE2683;
                      {
                        z2zE2683 = ztx_sig_span(znonce_f, zv_f);
                        if (have_exception) {
                          KILL(zTransaction)(&z3zE2560);
                          KILL(zTransaction)(&z3zE2561);
                          KILL(zTransaction)(&z3zE2562);
                          KILL(zTransaction)(&z3zE2563);
                          KILL(zTransaction)(&z3zE2564);
                          KILL(zTransaction)(&z3zE2565);
                          KILL(zTransaction)(&z3zE2566);
                          KILL(zTransaction)(&z3zE2567);
                          KILL(zTransaction)(&z3zE2568);
                          goto end_block_exception_3011;
                        }
                      }
                      {
                        z2zE2684 = ztx_signing_hash(zLegacyTx, z2zE2683, zv);
                        if (have_exception) {
                          KILL(zTransaction)(&z3zE2560);
                          KILL(zTransaction)(&z3zE2561);
                          KILL(zTransaction)(&z3zE2562);
                          KILL(zTransaction)(&z3zE2563);
                          KILL(zTransaction)(&z3zE2564);
                          KILL(zTransaction)(&z3zE2565);
                          KILL(zTransaction)(&z3zE2566);
                          KILL(zTransaction)(&z3zE2567);
                          KILL(zTransaction)(&z3zE2568);
                          goto end_block_exception_3011;
                        }
                      }
                    }
                    sail_u256 z2zE2685;
                    {
                      z2zE2685 = zrlp_ref_uint_word(zr_f);
                      if (have_exception) {
                        KILL(zTransaction)(&z3zE2560);
                        KILL(zTransaction)(&z3zE2561);
                        KILL(zTransaction)(&z3zE2562);
                        KILL(zTransaction)(&z3zE2563);
                        KILL(zTransaction)(&z3zE2564);
                        KILL(zTransaction)(&z3zE2565);
                        KILL(zTransaction)(&z3zE2566);
                        KILL(zTransaction)(&z3zE2567);
                        KILL(zTransaction)(&z3zE2568);
                        goto end_block_exception_3011;
                      }
                    }
                    sail_u256 z2zE2686;
                    {
                      z2zE2686 = zrlp_ref_uint_word(zs_f);
                      if (have_exception) {
                        KILL(zTransaction)(&z3zE2560);
                        KILL(zTransaction)(&z3zE2561);
                        KILL(zTransaction)(&z3zE2562);
                        KILL(zTransaction)(&z3zE2563);
                        KILL(zTransaction)(&z3zE2564);
                        KILL(zTransaction)(&z3zE2565);
                        KILL(zTransaction)(&z3zE2566);
                        KILL(zTransaction)(&z3zE2567);
                        KILL(zTransaction)(&z3zE2568);
                        goto end_block_exception_3011;
                      }
                    }
                    z3zE2568.zaccess_list_address_count = UINT64_C(0);
                    zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z3zE2571;
                    CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE2571);
                    COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE2568)->zaccess_list_addresses), z3zE2571);
                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE2571);
                    z3zE2568.zaccess_list_slot_count = UINT64_C(0);
                    zz5listz8z5structz0zzStorageKeyz9 z3zE2572;
                    CREATE(zz5listz8z5structz0zzStorageKeyz9)(&z3zE2572);
                    COPY(zz5listz8z5structz0zzStorageKeyz9)(&((&z3zE2568)->zaccess_list_slots), z3zE2572);
                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&z3zE2572);
                    z3zE2568.zauthorizzation_count = UINT64_C(0);
                    zz5listz8z5structz0zzAuthorizzzzationz9 z3zE2573;
                    CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&z3zE2573);
                    COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&((&z3zE2568)->zauthorizzations), z3zE2573);
                    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z3zE2573);
                    z3zE2568.zblob_hashes = zEMPTY_BLOB_HASHES;
                    z3zE2568.zchain_id = UINT64_C(0);
                    z3zE2568.zgas_limit = z2zE2676;
                    z3zE2568.zinput_src = z2zE2682;
                    z3zE2568.zis_create = z2zE2678;
                    z3zE2568.zmax_blob_fee = zZERO_WORD;
                    z3zE2568.zmax_fee = zgp;
                    z3zE2568.zmax_priority_fee = zgp;
                    z3zE2568.znonce = z2zE2675;
                    z3zE2568.zpubkey = zpubkey;
                    z3zE2568.zraw = ztx;
                    z3zE2568.zrecipient = z2zE2680;
                    z3zE2568.zsender = zsender;
                    z3zE2568.zsig_r = z2zE2685;
                    z3zE2568.zsig_s = z2zE2686;
                    z3zE2568.zsig_v = zv;
                    z3zE2568.zsigning_hash = z2zE2684;
                    z3zE2568.ztx_type = zLegacyTx;
                    z3zE2568.zvalue = z2zE2681;
                    goto finish_match_3008;
                  }
                case_3009: ;
                  sail_match_failure("decode_legacy_tx");
                finish_match_3008: ;
                  COPY(zTransaction)(&z3zE2567, z3zE2568);
                  KILL(zTransaction)(&z3zE2568);
                  goto finish_match_3006;
                }
              case_3007: ;
                sail_match_failure("decode_legacy_tx");
              finish_match_3006: ;
                COPY(zTransaction)(&z3zE2566, z3zE2567);
                KILL(zTransaction)(&z3zE2567);
                goto finish_match_3004;
              }
            case_3005: ;
              sail_match_failure("decode_legacy_tx");
            finish_match_3004: ;
              COPY(zTransaction)(&z3zE2565, z3zE2566);
              KILL(zTransaction)(&z3zE2566);
              goto finish_match_3002;
            }
          case_3003: ;
            sail_match_failure("decode_legacy_tx");
          finish_match_3002: ;
            COPY(zTransaction)(&z3zE2564, z3zE2565);
            KILL(zTransaction)(&z3zE2565);
            goto finish_match_3000;
          }
        case_3001: ;
          sail_match_failure("decode_legacy_tx");
        finish_match_3000: ;
          COPY(zTransaction)(&z3zE2563, z3zE2564);
          KILL(zTransaction)(&z3zE2564);
          goto finish_match_2998;
        }
      case_2999: ;
        sail_match_failure("decode_legacy_tx");
      finish_match_2998: ;
        COPY(zTransaction)(&z3zE2562, z3zE2563);
        KILL(zTransaction)(&z3zE2563);
        goto finish_match_2996;
      }
    case_2997: ;
      sail_match_failure("decode_legacy_tx");
    finish_match_2996: ;
      COPY(zTransaction)(&z3zE2561, z3zE2562);
      KILL(zTransaction)(&z3zE2562);
      goto finish_match_2994;
    }
  case_2995: ;
    sail_match_failure("decode_legacy_tx");
  finish_match_2994: ;
    COPY(zTransaction)(&z3zE2560, z3zE2561);
    KILL(zTransaction)(&z3zE2561);
    goto finish_match_2992;
  }
case_2993: ;
  sail_match_failure("decode_legacy_tx");
finish_match_2992: ;
  COPY(zTransaction)((*(&z8zE196)), z3zE2560);
  KILL(zTransaction)(&z3zE2560);
end_function_3010: ;
  goto end_function_3647;
end_block_exception_3011: ;
  goto end_function_3647;
end_function_3647: ;
}

void zdecode_access_list_tx(struct zTransaction *z8zE197, struct zByteSliceFields ztx, struct zByteSliceFields zpubkey, enum zFork zfork, sail_fixed_bytes_20 zsender, struct zByteSliceFields zfields)
{
  struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2641;
  {
    z2zE2641 = zrlp_cursor_pop(zfields);
    if (have_exception) {  goto end_block_exception_2991;  }
  }
  struct zTransaction z3zE2533;
  CREATE(zTransaction)(&z3zE2533);
  {
    struct zRlpFieldRef zchain_f;
    zchain_f = z2zE2641.ztup0;
    struct zByteSliceFields z1zE64;
    z1zE64 = z2zE2641.ztup1;
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2642;
    {
      z2zE2642 = zrlp_cursor_pop(z1zE64);
      if (have_exception) {
        KILL(zTransaction)(&z3zE2533);
        goto end_block_exception_2991;
      }
    }
    struct zTransaction z3zE2534;
    CREATE(zTransaction)(&z3zE2534);
    {
      struct zRlpFieldRef znonce_f;
      znonce_f = z2zE2642.ztup0;
      struct zByteSliceFields z1zE65;
      z1zE65 = z2zE2642.ztup1;
      struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2643;
      {
        z2zE2643 = zrlp_cursor_pop(z1zE65);
        if (have_exception) {
          KILL(zTransaction)(&z3zE2533);
          KILL(zTransaction)(&z3zE2534);
          goto end_block_exception_2991;
        }
      }
      struct zTransaction z3zE2535;
      CREATE(zTransaction)(&z3zE2535);
      {
        struct zRlpFieldRef zgp_f;
        zgp_f = z2zE2643.ztup0;
        struct zByteSliceFields z1zE66;
        z1zE66 = z2zE2643.ztup1;
        struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2644;
        {
          z2zE2644 = zrlp_cursor_pop(z1zE66);
          if (have_exception) {
            KILL(zTransaction)(&z3zE2533);
            KILL(zTransaction)(&z3zE2534);
            KILL(zTransaction)(&z3zE2535);
            goto end_block_exception_2991;
          }
        }
        struct zTransaction z3zE2536;
        CREATE(zTransaction)(&z3zE2536);
        {
          struct zRlpFieldRef zgas_f;
          zgas_f = z2zE2644.ztup0;
          struct zByteSliceFields z1zE67;
          z1zE67 = z2zE2644.ztup1;
          struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2645;
          {
            z2zE2645 = zrlp_cursor_pop(z1zE67);
            if (have_exception) {
              KILL(zTransaction)(&z3zE2533);
              KILL(zTransaction)(&z3zE2534);
              KILL(zTransaction)(&z3zE2535);
              KILL(zTransaction)(&z3zE2536);
              goto end_block_exception_2991;
            }
          }
          struct zTransaction z3zE2537;
          CREATE(zTransaction)(&z3zE2537);
          {
            struct zRlpFieldRef zto_f;
            zto_f = z2zE2645.ztup0;
            struct zByteSliceFields z1zE68;
            z1zE68 = z2zE2645.ztup1;
            struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2646;
            {
              z2zE2646 = zrlp_cursor_pop(z1zE68);
              if (have_exception) {
                KILL(zTransaction)(&z3zE2533);
                KILL(zTransaction)(&z3zE2534);
                KILL(zTransaction)(&z3zE2535);
                KILL(zTransaction)(&z3zE2536);
                KILL(zTransaction)(&z3zE2537);
                goto end_block_exception_2991;
              }
            }
            struct zTransaction z3zE2538;
            CREATE(zTransaction)(&z3zE2538);
            {
              struct zRlpFieldRef zvalue_f;
              zvalue_f = z2zE2646.ztup0;
              struct zByteSliceFields z1zE69;
              z1zE69 = z2zE2646.ztup1;
              struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2647;
              {
                z2zE2647 = zrlp_cursor_pop(z1zE69);
                if (have_exception) {
                  KILL(zTransaction)(&z3zE2533);
                  KILL(zTransaction)(&z3zE2534);
                  KILL(zTransaction)(&z3zE2535);
                  KILL(zTransaction)(&z3zE2536);
                  KILL(zTransaction)(&z3zE2537);
                  KILL(zTransaction)(&z3zE2538);
                  goto end_block_exception_2991;
                }
              }
              struct zTransaction z3zE2539;
              CREATE(zTransaction)(&z3zE2539);
              {
                struct zRlpFieldRef zdata_f;
                zdata_f = z2zE2647.ztup0;
                struct zByteSliceFields z1zE70;
                z1zE70 = z2zE2647.ztup1;
                struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2648;
                {
                  z2zE2648 = zrlp_cursor_pop(z1zE70);
                  if (have_exception) {
                    KILL(zTransaction)(&z3zE2533);
                    KILL(zTransaction)(&z3zE2534);
                    KILL(zTransaction)(&z3zE2535);
                    KILL(zTransaction)(&z3zE2536);
                    KILL(zTransaction)(&z3zE2537);
                    KILL(zTransaction)(&z3zE2538);
                    KILL(zTransaction)(&z3zE2539);
                    goto end_block_exception_2991;
                  }
                }
                struct zTransaction z3zE2540;
                CREATE(zTransaction)(&z3zE2540);
                {
                  struct zRlpFieldRef zal_f;
                  zal_f = z2zE2648.ztup0;
                  struct zByteSliceFields z1zE71;
                  z1zE71 = z2zE2648.ztup1;
                  struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2649;
                  {
                    z2zE2649 = zrlp_cursor_pop(z1zE71);
                    if (have_exception) {
                      KILL(zTransaction)(&z3zE2533);
                      KILL(zTransaction)(&z3zE2534);
                      KILL(zTransaction)(&z3zE2535);
                      KILL(zTransaction)(&z3zE2536);
                      KILL(zTransaction)(&z3zE2537);
                      KILL(zTransaction)(&z3zE2538);
                      KILL(zTransaction)(&z3zE2539);
                      KILL(zTransaction)(&z3zE2540);
                      goto end_block_exception_2991;
                    }
                  }
                  struct zTransaction z3zE2541;
                  CREATE(zTransaction)(&z3zE2541);
                  {
                    struct zRlpFieldRef zv_f;
                    zv_f = z2zE2649.ztup0;
                    struct zByteSliceFields z1zE72;
                    z1zE72 = z2zE2649.ztup1;
                    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2650;
                    {
                      z2zE2650 = zrlp_cursor_pop(z1zE72);
                      if (have_exception) {
                        KILL(zTransaction)(&z3zE2533);
                        KILL(zTransaction)(&z3zE2534);
                        KILL(zTransaction)(&z3zE2535);
                        KILL(zTransaction)(&z3zE2536);
                        KILL(zTransaction)(&z3zE2537);
                        KILL(zTransaction)(&z3zE2538);
                        KILL(zTransaction)(&z3zE2539);
                        KILL(zTransaction)(&z3zE2540);
                        KILL(zTransaction)(&z3zE2541);
                        goto end_block_exception_2991;
                      }
                    }
                    struct zTransaction z3zE2542;
                    CREATE(zTransaction)(&z3zE2542);
                    {
                      struct zRlpFieldRef zr_f;
                      zr_f = z2zE2650.ztup0;
                      struct zByteSliceFields z1zE73;
                      z1zE73 = z2zE2650.ztup1;
                      struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2651;
                      {
                        z2zE2651 = zrlp_cursor_pop(z1zE73);
                        if (have_exception) {
                          KILL(zTransaction)(&z3zE2533);
                          KILL(zTransaction)(&z3zE2534);
                          KILL(zTransaction)(&z3zE2535);
                          KILL(zTransaction)(&z3zE2536);
                          KILL(zTransaction)(&z3zE2537);
                          KILL(zTransaction)(&z3zE2538);
                          KILL(zTransaction)(&z3zE2539);
                          KILL(zTransaction)(&z3zE2540);
                          KILL(zTransaction)(&z3zE2541);
                          KILL(zTransaction)(&z3zE2542);
                          goto end_block_exception_2991;
                        }
                      }
                      struct zTransaction z3zE2543;
                      CREATE(zTransaction)(&z3zE2543);
                      {
                        struct zRlpFieldRef zs_f;
                        zs_f = z2zE2651.ztup0;
                        struct zByteSliceFields z1zE74;
                        z1zE74 = z2zE2651.ztup1;
                        unit z3zE2544;
                        {
                          z3zE2544 = zrlp_cursor_expect_end(z1zE74);
                          if (have_exception) {
                            KILL(zTransaction)(&z3zE2533);
                            KILL(zTransaction)(&z3zE2534);
                            KILL(zTransaction)(&z3zE2535);
                            KILL(zTransaction)(&z3zE2536);
                            KILL(zTransaction)(&z3zE2537);
                            KILL(zTransaction)(&z3zE2538);
                            KILL(zTransaction)(&z3zE2539);
                            KILL(zTransaction)(&z3zE2540);
                            KILL(zTransaction)(&z3zE2541);
                            KILL(zTransaction)(&z3zE2542);
                            KILL(zTransaction)(&z3zE2543);
                            goto end_block_exception_2991;
                          }
                        }
                        sail_u256 zv;
                        {
                          zv = zrlp_ref_word(zv_f);
                          if (have_exception) {
                            KILL(zTransaction)(&z3zE2533);
                            KILL(zTransaction)(&z3zE2534);
                            KILL(zTransaction)(&z3zE2535);
                            KILL(zTransaction)(&z3zE2536);
                            KILL(zTransaction)(&z3zE2537);
                            KILL(zTransaction)(&z3zE2538);
                            KILL(zTransaction)(&z3zE2539);
                            KILL(zTransaction)(&z3zE2540);
                            KILL(zTransaction)(&z3zE2541);
                            KILL(zTransaction)(&z3zE2542);
                            KILL(zTransaction)(&z3zE2543);
                            goto end_block_exception_2991;
                          }
                        }
                        sail_u256 zgp;
                        {
                          zgp = zrlp_ref_uint_word(zgp_f);
                          if (have_exception) {
                            KILL(zTransaction)(&z3zE2533);
                            KILL(zTransaction)(&z3zE2534);
                            KILL(zTransaction)(&z3zE2535);
                            KILL(zTransaction)(&z3zE2536);
                            KILL(zTransaction)(&z3zE2537);
                            KILL(zTransaction)(&z3zE2538);
                            KILL(zTransaction)(&z3zE2539);
                            KILL(zTransaction)(&z3zE2540);
                            KILL(zTransaction)(&z3zE2541);
                            KILL(zTransaction)(&z3zE2542);
                            KILL(zTransaction)(&z3zE2543);
                            goto end_block_exception_2991;
                          }
                        }
                        struct ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9 z2zE2652;
                        CREATE(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2652);
                        {
                          zdecode_access_list(&z2zE2652, zal_f);
                          if (have_exception) {
                            KILL(zTransaction)(&z3zE2533);
                            KILL(zTransaction)(&z3zE2534);
                            KILL(zTransaction)(&z3zE2535);
                            KILL(zTransaction)(&z3zE2536);
                            KILL(zTransaction)(&z3zE2537);
                            KILL(zTransaction)(&z3zE2538);
                            KILL(zTransaction)(&z3zE2539);
                            KILL(zTransaction)(&z3zE2540);
                            KILL(zTransaction)(&z3zE2541);
                            KILL(zTransaction)(&z3zE2542);
                            KILL(zTransaction)(&z3zE2543);
                            KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2652);
                            goto end_block_exception_2991;
                          }
                        }
                        struct zTransaction z3zE2545;
                        CREATE(zTransaction)(&z3zE2545);
                        {
                          zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 zal_addrs;
                          CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                          COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs, z2zE2652.ztup0);
                          zz5listz8z5structz0zzStorageKeyz9 zal_slots;
                          CREATE(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                          COPY(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots, z2zE2652.ztup1);
                          uint64_t zal_addr_count;
                          zal_addr_count = z2zE2652.ztup2;
                          uint64_t zal_slot_count;
                          zal_slot_count = z2zE2652.ztup3;
                          sail_u256 z2zE2653;
                          {
                            z2zE2653 = zrlp_ref_uint_word(znonce_f);
                            if (have_exception) {
                              KILL(zTransaction)(&z3zE2533);
                              KILL(zTransaction)(&z3zE2534);
                              KILL(zTransaction)(&z3zE2535);
                              KILL(zTransaction)(&z3zE2536);
                              KILL(zTransaction)(&z3zE2537);
                              KILL(zTransaction)(&z3zE2538);
                              KILL(zTransaction)(&z3zE2539);
                              KILL(zTransaction)(&z3zE2540);
                              KILL(zTransaction)(&z3zE2541);
                              KILL(zTransaction)(&z3zE2542);
                              KILL(zTransaction)(&z3zE2543);
                              KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2652);
                              KILL(zTransaction)(&z3zE2545);
                              KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                              KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                              goto end_block_exception_2991;
                            }
                          }
                          uint64_t z2zE2654;
                          {
                            z2zE2654 = zrlp_ref_uint64(zchain_f);
                            if (have_exception) {
                              KILL(zTransaction)(&z3zE2533);
                              KILL(zTransaction)(&z3zE2534);
                              KILL(zTransaction)(&z3zE2535);
                              KILL(zTransaction)(&z3zE2536);
                              KILL(zTransaction)(&z3zE2537);
                              KILL(zTransaction)(&z3zE2538);
                              KILL(zTransaction)(&z3zE2539);
                              KILL(zTransaction)(&z3zE2540);
                              KILL(zTransaction)(&z3zE2541);
                              KILL(zTransaction)(&z3zE2542);
                              KILL(zTransaction)(&z3zE2543);
                              KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2652);
                              KILL(zTransaction)(&z3zE2545);
                              KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                              KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                              goto end_block_exception_2991;
                            }
                          }
                          uint64_t z2zE2655;
                          {
                            z2zE2655 = zrlp_ref_gas(zgas_f, zfork);
                            if (have_exception) {
                              KILL(zTransaction)(&z3zE2533);
                              KILL(zTransaction)(&z3zE2534);
                              KILL(zTransaction)(&z3zE2535);
                              KILL(zTransaction)(&z3zE2536);
                              KILL(zTransaction)(&z3zE2537);
                              KILL(zTransaction)(&z3zE2538);
                              KILL(zTransaction)(&z3zE2539);
                              KILL(zTransaction)(&z3zE2540);
                              KILL(zTransaction)(&z3zE2541);
                              KILL(zTransaction)(&z3zE2542);
                              KILL(zTransaction)(&z3zE2543);
                              KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2652);
                              KILL(zTransaction)(&z3zE2545);
                              KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                              KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                              goto end_block_exception_2991;
                            }
                          }
                          bool z2zE2657;
                          {
                            uint64_t z2zE2656;
                            z2zE2656 = zto_f.zcontent_len;
                            z2zE2657 = (z2zE2656 == UINT64_C(0));
                          }
                          sail_fixed_bytes_20 z2zE2659;
                          {
                            sail_u256 z2zE2658;
                            {
                              z2zE2658 = zrlp_ref_word(zto_f);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE2533);
                                KILL(zTransaction)(&z3zE2534);
                                KILL(zTransaction)(&z3zE2535);
                                KILL(zTransaction)(&z3zE2536);
                                KILL(zTransaction)(&z3zE2537);
                                KILL(zTransaction)(&z3zE2538);
                                KILL(zTransaction)(&z3zE2539);
                                KILL(zTransaction)(&z3zE2540);
                                KILL(zTransaction)(&z3zE2541);
                                KILL(zTransaction)(&z3zE2542);
                                KILL(zTransaction)(&z3zE2543);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2652);
                                KILL(zTransaction)(&z3zE2545);
                                KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                goto end_block_exception_2991;
                              }
                            }
                            z2zE2659 = evmsail_word_to_address(z2zE2658);
                          }
                          sail_u256 z2zE2660;
                          {
                            z2zE2660 = zrlp_ref_uint_word(zvalue_f);
                            if (have_exception) {
                              KILL(zTransaction)(&z3zE2533);
                              KILL(zTransaction)(&z3zE2534);
                              KILL(zTransaction)(&z3zE2535);
                              KILL(zTransaction)(&z3zE2536);
                              KILL(zTransaction)(&z3zE2537);
                              KILL(zTransaction)(&z3zE2538);
                              KILL(zTransaction)(&z3zE2539);
                              KILL(zTransaction)(&z3zE2540);
                              KILL(zTransaction)(&z3zE2541);
                              KILL(zTransaction)(&z3zE2542);
                              KILL(zTransaction)(&z3zE2543);
                              KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2652);
                              KILL(zTransaction)(&z3zE2545);
                              KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                              KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                              goto end_block_exception_2991;
                            }
                          }
                          struct zByteSliceFields z2zE2661;
                          {
                            z2zE2661 = ztx_input_span(zdata_f);
                            if (have_exception) {
                              KILL(zTransaction)(&z3zE2533);
                              KILL(zTransaction)(&z3zE2534);
                              KILL(zTransaction)(&z3zE2535);
                              KILL(zTransaction)(&z3zE2536);
                              KILL(zTransaction)(&z3zE2537);
                              KILL(zTransaction)(&z3zE2538);
                              KILL(zTransaction)(&z3zE2539);
                              KILL(zTransaction)(&z3zE2540);
                              KILL(zTransaction)(&z3zE2541);
                              KILL(zTransaction)(&z3zE2542);
                              KILL(zTransaction)(&z3zE2543);
                              KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2652);
                              KILL(zTransaction)(&z3zE2545);
                              KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                              KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                              goto end_block_exception_2991;
                            }
                          }
                          sail_fixed_bytes_32 z2zE2663;
                          {
                            struct zByteSliceFields z2zE2662;
                            {
                              z2zE2662 = ztx_sig_span(zchain_f, zv_f);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE2533);
                                KILL(zTransaction)(&z3zE2534);
                                KILL(zTransaction)(&z3zE2535);
                                KILL(zTransaction)(&z3zE2536);
                                KILL(zTransaction)(&z3zE2537);
                                KILL(zTransaction)(&z3zE2538);
                                KILL(zTransaction)(&z3zE2539);
                                KILL(zTransaction)(&z3zE2540);
                                KILL(zTransaction)(&z3zE2541);
                                KILL(zTransaction)(&z3zE2542);
                                KILL(zTransaction)(&z3zE2543);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2652);
                                KILL(zTransaction)(&z3zE2545);
                                KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                goto end_block_exception_2991;
                              }
                            }
                            {
                              z2zE2663 = ztx_signing_hash(zAccessListTx, z2zE2662, zv);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE2533);
                                KILL(zTransaction)(&z3zE2534);
                                KILL(zTransaction)(&z3zE2535);
                                KILL(zTransaction)(&z3zE2536);
                                KILL(zTransaction)(&z3zE2537);
                                KILL(zTransaction)(&z3zE2538);
                                KILL(zTransaction)(&z3zE2539);
                                KILL(zTransaction)(&z3zE2540);
                                KILL(zTransaction)(&z3zE2541);
                                KILL(zTransaction)(&z3zE2542);
                                KILL(zTransaction)(&z3zE2543);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2652);
                                KILL(zTransaction)(&z3zE2545);
                                KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                goto end_block_exception_2991;
                              }
                            }
                          }
                          sail_u256 z2zE2664;
                          {
                            z2zE2664 = zrlp_ref_uint_word(zr_f);
                            if (have_exception) {
                              KILL(zTransaction)(&z3zE2533);
                              KILL(zTransaction)(&z3zE2534);
                              KILL(zTransaction)(&z3zE2535);
                              KILL(zTransaction)(&z3zE2536);
                              KILL(zTransaction)(&z3zE2537);
                              KILL(zTransaction)(&z3zE2538);
                              KILL(zTransaction)(&z3zE2539);
                              KILL(zTransaction)(&z3zE2540);
                              KILL(zTransaction)(&z3zE2541);
                              KILL(zTransaction)(&z3zE2542);
                              KILL(zTransaction)(&z3zE2543);
                              KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2652);
                              KILL(zTransaction)(&z3zE2545);
                              KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                              KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                              goto end_block_exception_2991;
                            }
                          }
                          sail_u256 z2zE2665;
                          {
                            z2zE2665 = zrlp_ref_uint_word(zs_f);
                            if (have_exception) {
                              KILL(zTransaction)(&z3zE2533);
                              KILL(zTransaction)(&z3zE2534);
                              KILL(zTransaction)(&z3zE2535);
                              KILL(zTransaction)(&z3zE2536);
                              KILL(zTransaction)(&z3zE2537);
                              KILL(zTransaction)(&z3zE2538);
                              KILL(zTransaction)(&z3zE2539);
                              KILL(zTransaction)(&z3zE2540);
                              KILL(zTransaction)(&z3zE2541);
                              KILL(zTransaction)(&z3zE2542);
                              KILL(zTransaction)(&z3zE2543);
                              KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2652);
                              KILL(zTransaction)(&z3zE2545);
                              KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                              KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                              goto end_block_exception_2991;
                            }
                          }
                          z3zE2545.zaccess_list_address_count = zal_addr_count;
                          COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE2545)->zaccess_list_addresses), zal_addrs);
                          z3zE2545.zaccess_list_slot_count = zal_slot_count;
                          COPY(zz5listz8z5structz0zzStorageKeyz9)(&((&z3zE2545)->zaccess_list_slots), zal_slots);
                          z3zE2545.zauthorizzation_count = UINT64_C(0);
                          zz5listz8z5structz0zzAuthorizzzzationz9 z3zE2547;
                          CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&z3zE2547);
                          COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&((&z3zE2545)->zauthorizzations), z3zE2547);
                          KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z3zE2547);
                          z3zE2545.zblob_hashes = zEMPTY_BLOB_HASHES;
                          z3zE2545.zchain_id = z2zE2654;
                          z3zE2545.zgas_limit = z2zE2655;
                          z3zE2545.zinput_src = z2zE2661;
                          z3zE2545.zis_create = z2zE2657;
                          z3zE2545.zmax_blob_fee = zZERO_WORD;
                          z3zE2545.zmax_fee = zgp;
                          z3zE2545.zmax_priority_fee = zgp;
                          z3zE2545.znonce = z2zE2653;
                          z3zE2545.zpubkey = zpubkey;
                          z3zE2545.zraw = ztx;
                          z3zE2545.zrecipient = z2zE2659;
                          z3zE2545.zsender = zsender;
                          z3zE2545.zsig_r = z2zE2664;
                          z3zE2545.zsig_s = z2zE2665;
                          z3zE2545.zsig_v = zv;
                          z3zE2545.zsigning_hash = z2zE2663;
                          z3zE2545.ztx_type = zAccessListTx;
                          z3zE2545.zvalue = z2zE2660;
                          KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                          KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                          goto finish_match_2988;
                        }
                      case_2989: ;
                        sail_match_failure("decode_access_list_tx");
                      finish_match_2988: ;
                        COPY(zTransaction)(&z3zE2543, z3zE2545);
                        KILL(zTransaction)(&z3zE2545);
                        KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2652);
                        goto finish_match_2986;
                      }
                    case_2987: ;
                      sail_match_failure("decode_access_list_tx");
                    finish_match_2986: ;
                      COPY(zTransaction)(&z3zE2542, z3zE2543);
                      KILL(zTransaction)(&z3zE2543);
                      goto finish_match_2984;
                    }
                  case_2985: ;
                    sail_match_failure("decode_access_list_tx");
                  finish_match_2984: ;
                    COPY(zTransaction)(&z3zE2541, z3zE2542);
                    KILL(zTransaction)(&z3zE2542);
                    goto finish_match_2982;
                  }
                case_2983: ;
                  sail_match_failure("decode_access_list_tx");
                finish_match_2982: ;
                  COPY(zTransaction)(&z3zE2540, z3zE2541);
                  KILL(zTransaction)(&z3zE2541);
                  goto finish_match_2980;
                }
              case_2981: ;
                sail_match_failure("decode_access_list_tx");
              finish_match_2980: ;
                COPY(zTransaction)(&z3zE2539, z3zE2540);
                KILL(zTransaction)(&z3zE2540);
                goto finish_match_2978;
              }
            case_2979: ;
              sail_match_failure("decode_access_list_tx");
            finish_match_2978: ;
              COPY(zTransaction)(&z3zE2538, z3zE2539);
              KILL(zTransaction)(&z3zE2539);
              goto finish_match_2976;
            }
          case_2977: ;
            sail_match_failure("decode_access_list_tx");
          finish_match_2976: ;
            COPY(zTransaction)(&z3zE2537, z3zE2538);
            KILL(zTransaction)(&z3zE2538);
            goto finish_match_2974;
          }
        case_2975: ;
          sail_match_failure("decode_access_list_tx");
        finish_match_2974: ;
          COPY(zTransaction)(&z3zE2536, z3zE2537);
          KILL(zTransaction)(&z3zE2537);
          goto finish_match_2972;
        }
      case_2973: ;
        sail_match_failure("decode_access_list_tx");
      finish_match_2972: ;
        COPY(zTransaction)(&z3zE2535, z3zE2536);
        KILL(zTransaction)(&z3zE2536);
        goto finish_match_2970;
      }
    case_2971: ;
      sail_match_failure("decode_access_list_tx");
    finish_match_2970: ;
      COPY(zTransaction)(&z3zE2534, z3zE2535);
      KILL(zTransaction)(&z3zE2535);
      goto finish_match_2968;
    }
  case_2969: ;
    sail_match_failure("decode_access_list_tx");
  finish_match_2968: ;
    COPY(zTransaction)(&z3zE2533, z3zE2534);
    KILL(zTransaction)(&z3zE2534);
    goto finish_match_2966;
  }
case_2967: ;
  sail_match_failure("decode_access_list_tx");
finish_match_2966: ;
  COPY(zTransaction)((*(&z8zE197)), z3zE2533);
  KILL(zTransaction)(&z3zE2533);
end_function_2990: ;
  goto end_function_3646;
end_block_exception_2991: ;
  goto end_function_3646;
end_function_3646: ;
}

void zdecode_fee_market_tx(struct zTransaction *z8zE198, struct zByteSliceFields ztx, struct zByteSliceFields zpubkey, enum zFork zfork, sail_fixed_bytes_20 zsender, struct zByteSliceFields zfields)
{
  struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2613;
  {
    z2zE2613 = zrlp_cursor_pop(zfields);
    if (have_exception) {  goto end_block_exception_2965;  }
  }
  struct zTransaction z3zE2504;
  CREATE(zTransaction)(&z3zE2504);
  {
    struct zRlpFieldRef zchain_f;
    zchain_f = z2zE2613.ztup0;
    struct zByteSliceFields z1zE52;
    z1zE52 = z2zE2613.ztup1;
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2614;
    {
      z2zE2614 = zrlp_cursor_pop(z1zE52);
      if (have_exception) {
        KILL(zTransaction)(&z3zE2504);
        goto end_block_exception_2965;
      }
    }
    struct zTransaction z3zE2505;
    CREATE(zTransaction)(&z3zE2505);
    {
      struct zRlpFieldRef znonce_f;
      znonce_f = z2zE2614.ztup0;
      struct zByteSliceFields z1zE53;
      z1zE53 = z2zE2614.ztup1;
      struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2615;
      {
        z2zE2615 = zrlp_cursor_pop(z1zE53);
        if (have_exception) {
          KILL(zTransaction)(&z3zE2504);
          KILL(zTransaction)(&z3zE2505);
          goto end_block_exception_2965;
        }
      }
      struct zTransaction z3zE2506;
      CREATE(zTransaction)(&z3zE2506);
      {
        struct zRlpFieldRef zmp_f;
        zmp_f = z2zE2615.ztup0;
        struct zByteSliceFields z1zE54;
        z1zE54 = z2zE2615.ztup1;
        struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2616;
        {
          z2zE2616 = zrlp_cursor_pop(z1zE54);
          if (have_exception) {
            KILL(zTransaction)(&z3zE2504);
            KILL(zTransaction)(&z3zE2505);
            KILL(zTransaction)(&z3zE2506);
            goto end_block_exception_2965;
          }
        }
        struct zTransaction z3zE2507;
        CREATE(zTransaction)(&z3zE2507);
        {
          struct zRlpFieldRef zmf_f;
          zmf_f = z2zE2616.ztup0;
          struct zByteSliceFields z1zE55;
          z1zE55 = z2zE2616.ztup1;
          struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2617;
          {
            z2zE2617 = zrlp_cursor_pop(z1zE55);
            if (have_exception) {
              KILL(zTransaction)(&z3zE2504);
              KILL(zTransaction)(&z3zE2505);
              KILL(zTransaction)(&z3zE2506);
              KILL(zTransaction)(&z3zE2507);
              goto end_block_exception_2965;
            }
          }
          struct zTransaction z3zE2508;
          CREATE(zTransaction)(&z3zE2508);
          {
            struct zRlpFieldRef zgas_f;
            zgas_f = z2zE2617.ztup0;
            struct zByteSliceFields z1zE56;
            z1zE56 = z2zE2617.ztup1;
            struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2618;
            {
              z2zE2618 = zrlp_cursor_pop(z1zE56);
              if (have_exception) {
                KILL(zTransaction)(&z3zE2504);
                KILL(zTransaction)(&z3zE2505);
                KILL(zTransaction)(&z3zE2506);
                KILL(zTransaction)(&z3zE2507);
                KILL(zTransaction)(&z3zE2508);
                goto end_block_exception_2965;
              }
            }
            struct zTransaction z3zE2509;
            CREATE(zTransaction)(&z3zE2509);
            {
              struct zRlpFieldRef zto_f;
              zto_f = z2zE2618.ztup0;
              struct zByteSliceFields z1zE57;
              z1zE57 = z2zE2618.ztup1;
              struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2619;
              {
                z2zE2619 = zrlp_cursor_pop(z1zE57);
                if (have_exception) {
                  KILL(zTransaction)(&z3zE2504);
                  KILL(zTransaction)(&z3zE2505);
                  KILL(zTransaction)(&z3zE2506);
                  KILL(zTransaction)(&z3zE2507);
                  KILL(zTransaction)(&z3zE2508);
                  KILL(zTransaction)(&z3zE2509);
                  goto end_block_exception_2965;
                }
              }
              struct zTransaction z3zE2510;
              CREATE(zTransaction)(&z3zE2510);
              {
                struct zRlpFieldRef zvalue_f;
                zvalue_f = z2zE2619.ztup0;
                struct zByteSliceFields z1zE58;
                z1zE58 = z2zE2619.ztup1;
                struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2620;
                {
                  z2zE2620 = zrlp_cursor_pop(z1zE58);
                  if (have_exception) {
                    KILL(zTransaction)(&z3zE2504);
                    KILL(zTransaction)(&z3zE2505);
                    KILL(zTransaction)(&z3zE2506);
                    KILL(zTransaction)(&z3zE2507);
                    KILL(zTransaction)(&z3zE2508);
                    KILL(zTransaction)(&z3zE2509);
                    KILL(zTransaction)(&z3zE2510);
                    goto end_block_exception_2965;
                  }
                }
                struct zTransaction z3zE2511;
                CREATE(zTransaction)(&z3zE2511);
                {
                  struct zRlpFieldRef zdata_f;
                  zdata_f = z2zE2620.ztup0;
                  struct zByteSliceFields z1zE59;
                  z1zE59 = z2zE2620.ztup1;
                  struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2621;
                  {
                    z2zE2621 = zrlp_cursor_pop(z1zE59);
                    if (have_exception) {
                      KILL(zTransaction)(&z3zE2504);
                      KILL(zTransaction)(&z3zE2505);
                      KILL(zTransaction)(&z3zE2506);
                      KILL(zTransaction)(&z3zE2507);
                      KILL(zTransaction)(&z3zE2508);
                      KILL(zTransaction)(&z3zE2509);
                      KILL(zTransaction)(&z3zE2510);
                      KILL(zTransaction)(&z3zE2511);
                      goto end_block_exception_2965;
                    }
                  }
                  struct zTransaction z3zE2512;
                  CREATE(zTransaction)(&z3zE2512);
                  {
                    struct zRlpFieldRef zal_f;
                    zal_f = z2zE2621.ztup0;
                    struct zByteSliceFields z1zE60;
                    z1zE60 = z2zE2621.ztup1;
                    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2622;
                    {
                      z2zE2622 = zrlp_cursor_pop(z1zE60);
                      if (have_exception) {
                        KILL(zTransaction)(&z3zE2504);
                        KILL(zTransaction)(&z3zE2505);
                        KILL(zTransaction)(&z3zE2506);
                        KILL(zTransaction)(&z3zE2507);
                        KILL(zTransaction)(&z3zE2508);
                        KILL(zTransaction)(&z3zE2509);
                        KILL(zTransaction)(&z3zE2510);
                        KILL(zTransaction)(&z3zE2511);
                        KILL(zTransaction)(&z3zE2512);
                        goto end_block_exception_2965;
                      }
                    }
                    struct zTransaction z3zE2513;
                    CREATE(zTransaction)(&z3zE2513);
                    {
                      struct zRlpFieldRef zv_f;
                      zv_f = z2zE2622.ztup0;
                      struct zByteSliceFields z1zE61;
                      z1zE61 = z2zE2622.ztup1;
                      struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2623;
                      {
                        z2zE2623 = zrlp_cursor_pop(z1zE61);
                        if (have_exception) {
                          KILL(zTransaction)(&z3zE2504);
                          KILL(zTransaction)(&z3zE2505);
                          KILL(zTransaction)(&z3zE2506);
                          KILL(zTransaction)(&z3zE2507);
                          KILL(zTransaction)(&z3zE2508);
                          KILL(zTransaction)(&z3zE2509);
                          KILL(zTransaction)(&z3zE2510);
                          KILL(zTransaction)(&z3zE2511);
                          KILL(zTransaction)(&z3zE2512);
                          KILL(zTransaction)(&z3zE2513);
                          goto end_block_exception_2965;
                        }
                      }
                      struct zTransaction z3zE2514;
                      CREATE(zTransaction)(&z3zE2514);
                      {
                        struct zRlpFieldRef zr_f;
                        zr_f = z2zE2623.ztup0;
                        struct zByteSliceFields z1zE62;
                        z1zE62 = z2zE2623.ztup1;
                        struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2624;
                        {
                          z2zE2624 = zrlp_cursor_pop(z1zE62);
                          if (have_exception) {
                            KILL(zTransaction)(&z3zE2504);
                            KILL(zTransaction)(&z3zE2505);
                            KILL(zTransaction)(&z3zE2506);
                            KILL(zTransaction)(&z3zE2507);
                            KILL(zTransaction)(&z3zE2508);
                            KILL(zTransaction)(&z3zE2509);
                            KILL(zTransaction)(&z3zE2510);
                            KILL(zTransaction)(&z3zE2511);
                            KILL(zTransaction)(&z3zE2512);
                            KILL(zTransaction)(&z3zE2513);
                            KILL(zTransaction)(&z3zE2514);
                            goto end_block_exception_2965;
                          }
                        }
                        struct zTransaction z3zE2515;
                        CREATE(zTransaction)(&z3zE2515);
                        {
                          struct zRlpFieldRef zs_f;
                          zs_f = z2zE2624.ztup0;
                          struct zByteSliceFields z1zE63;
                          z1zE63 = z2zE2624.ztup1;
                          unit z3zE2516;
                          {
                            z3zE2516 = zrlp_cursor_expect_end(z1zE63);
                            if (have_exception) {
                              KILL(zTransaction)(&z3zE2504);
                              KILL(zTransaction)(&z3zE2505);
                              KILL(zTransaction)(&z3zE2506);
                              KILL(zTransaction)(&z3zE2507);
                              KILL(zTransaction)(&z3zE2508);
                              KILL(zTransaction)(&z3zE2509);
                              KILL(zTransaction)(&z3zE2510);
                              KILL(zTransaction)(&z3zE2511);
                              KILL(zTransaction)(&z3zE2512);
                              KILL(zTransaction)(&z3zE2513);
                              KILL(zTransaction)(&z3zE2514);
                              KILL(zTransaction)(&z3zE2515);
                              goto end_block_exception_2965;
                            }
                          }
                          sail_u256 zv;
                          {
                            zv = zrlp_ref_word(zv_f);
                            if (have_exception) {
                              KILL(zTransaction)(&z3zE2504);
                              KILL(zTransaction)(&z3zE2505);
                              KILL(zTransaction)(&z3zE2506);
                              KILL(zTransaction)(&z3zE2507);
                              KILL(zTransaction)(&z3zE2508);
                              KILL(zTransaction)(&z3zE2509);
                              KILL(zTransaction)(&z3zE2510);
                              KILL(zTransaction)(&z3zE2511);
                              KILL(zTransaction)(&z3zE2512);
                              KILL(zTransaction)(&z3zE2513);
                              KILL(zTransaction)(&z3zE2514);
                              KILL(zTransaction)(&z3zE2515);
                              goto end_block_exception_2965;
                            }
                          }
                          struct ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9 z2zE2625;
                          CREATE(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2625);
                          {
                            zdecode_access_list(&z2zE2625, zal_f);
                            if (have_exception) {
                              KILL(zTransaction)(&z3zE2504);
                              KILL(zTransaction)(&z3zE2505);
                              KILL(zTransaction)(&z3zE2506);
                              KILL(zTransaction)(&z3zE2507);
                              KILL(zTransaction)(&z3zE2508);
                              KILL(zTransaction)(&z3zE2509);
                              KILL(zTransaction)(&z3zE2510);
                              KILL(zTransaction)(&z3zE2511);
                              KILL(zTransaction)(&z3zE2512);
                              KILL(zTransaction)(&z3zE2513);
                              KILL(zTransaction)(&z3zE2514);
                              KILL(zTransaction)(&z3zE2515);
                              KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2625);
                              goto end_block_exception_2965;
                            }
                          }
                          struct zTransaction z3zE2517;
                          CREATE(zTransaction)(&z3zE2517);
                          {
                            zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 zal_addrs;
                            CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                            COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs, z2zE2625.ztup0);
                            zz5listz8z5structz0zzStorageKeyz9 zal_slots;
                            CREATE(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                            COPY(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots, z2zE2625.ztup1);
                            uint64_t zal_addr_count;
                            zal_addr_count = z2zE2625.ztup2;
                            uint64_t zal_slot_count;
                            zal_slot_count = z2zE2625.ztup3;
                            sail_u256 z2zE2626;
                            {
                              z2zE2626 = zrlp_ref_uint_word(znonce_f);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE2504);
                                KILL(zTransaction)(&z3zE2505);
                                KILL(zTransaction)(&z3zE2506);
                                KILL(zTransaction)(&z3zE2507);
                                KILL(zTransaction)(&z3zE2508);
                                KILL(zTransaction)(&z3zE2509);
                                KILL(zTransaction)(&z3zE2510);
                                KILL(zTransaction)(&z3zE2511);
                                KILL(zTransaction)(&z3zE2512);
                                KILL(zTransaction)(&z3zE2513);
                                KILL(zTransaction)(&z3zE2514);
                                KILL(zTransaction)(&z3zE2515);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2625);
                                KILL(zTransaction)(&z3zE2517);
                                KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                goto end_block_exception_2965;
                              }
                            }
                            uint64_t z2zE2627;
                            {
                              z2zE2627 = zrlp_ref_uint64(zchain_f);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE2504);
                                KILL(zTransaction)(&z3zE2505);
                                KILL(zTransaction)(&z3zE2506);
                                KILL(zTransaction)(&z3zE2507);
                                KILL(zTransaction)(&z3zE2508);
                                KILL(zTransaction)(&z3zE2509);
                                KILL(zTransaction)(&z3zE2510);
                                KILL(zTransaction)(&z3zE2511);
                                KILL(zTransaction)(&z3zE2512);
                                KILL(zTransaction)(&z3zE2513);
                                KILL(zTransaction)(&z3zE2514);
                                KILL(zTransaction)(&z3zE2515);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2625);
                                KILL(zTransaction)(&z3zE2517);
                                KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                goto end_block_exception_2965;
                              }
                            }
                            uint64_t z2zE2628;
                            {
                              z2zE2628 = zrlp_ref_gas(zgas_f, zfork);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE2504);
                                KILL(zTransaction)(&z3zE2505);
                                KILL(zTransaction)(&z3zE2506);
                                KILL(zTransaction)(&z3zE2507);
                                KILL(zTransaction)(&z3zE2508);
                                KILL(zTransaction)(&z3zE2509);
                                KILL(zTransaction)(&z3zE2510);
                                KILL(zTransaction)(&z3zE2511);
                                KILL(zTransaction)(&z3zE2512);
                                KILL(zTransaction)(&z3zE2513);
                                KILL(zTransaction)(&z3zE2514);
                                KILL(zTransaction)(&z3zE2515);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2625);
                                KILL(zTransaction)(&z3zE2517);
                                KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                goto end_block_exception_2965;
                              }
                            }
                            bool z2zE2630;
                            {
                              uint64_t z2zE2629;
                              z2zE2629 = zto_f.zcontent_len;
                              z2zE2630 = (z2zE2629 == UINT64_C(0));
                            }
                            sail_fixed_bytes_20 z2zE2632;
                            {
                              sail_u256 z2zE2631;
                              {
                                z2zE2631 = zrlp_ref_word(zto_f);
                                if (have_exception) {
                                  KILL(zTransaction)(&z3zE2504);
                                  KILL(zTransaction)(&z3zE2505);
                                  KILL(zTransaction)(&z3zE2506);
                                  KILL(zTransaction)(&z3zE2507);
                                  KILL(zTransaction)(&z3zE2508);
                                  KILL(zTransaction)(&z3zE2509);
                                  KILL(zTransaction)(&z3zE2510);
                                  KILL(zTransaction)(&z3zE2511);
                                  KILL(zTransaction)(&z3zE2512);
                                  KILL(zTransaction)(&z3zE2513);
                                  KILL(zTransaction)(&z3zE2514);
                                  KILL(zTransaction)(&z3zE2515);
                                  KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2625);
                                  KILL(zTransaction)(&z3zE2517);
                                  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                  KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                  goto end_block_exception_2965;
                                }
                              }
                              z2zE2632 = evmsail_word_to_address(z2zE2631);
                            }
                            sail_u256 z2zE2633;
                            {
                              z2zE2633 = zrlp_ref_uint_word(zvalue_f);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE2504);
                                KILL(zTransaction)(&z3zE2505);
                                KILL(zTransaction)(&z3zE2506);
                                KILL(zTransaction)(&z3zE2507);
                                KILL(zTransaction)(&z3zE2508);
                                KILL(zTransaction)(&z3zE2509);
                                KILL(zTransaction)(&z3zE2510);
                                KILL(zTransaction)(&z3zE2511);
                                KILL(zTransaction)(&z3zE2512);
                                KILL(zTransaction)(&z3zE2513);
                                KILL(zTransaction)(&z3zE2514);
                                KILL(zTransaction)(&z3zE2515);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2625);
                                KILL(zTransaction)(&z3zE2517);
                                KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                goto end_block_exception_2965;
                              }
                            }
                            struct zByteSliceFields z2zE2634;
                            {
                              z2zE2634 = ztx_input_span(zdata_f);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE2504);
                                KILL(zTransaction)(&z3zE2505);
                                KILL(zTransaction)(&z3zE2506);
                                KILL(zTransaction)(&z3zE2507);
                                KILL(zTransaction)(&z3zE2508);
                                KILL(zTransaction)(&z3zE2509);
                                KILL(zTransaction)(&z3zE2510);
                                KILL(zTransaction)(&z3zE2511);
                                KILL(zTransaction)(&z3zE2512);
                                KILL(zTransaction)(&z3zE2513);
                                KILL(zTransaction)(&z3zE2514);
                                KILL(zTransaction)(&z3zE2515);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2625);
                                KILL(zTransaction)(&z3zE2517);
                                KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                goto end_block_exception_2965;
                              }
                            }
                            sail_u256 z2zE2635;
                            {
                              z2zE2635 = zrlp_ref_uint_word(zmf_f);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE2504);
                                KILL(zTransaction)(&z3zE2505);
                                KILL(zTransaction)(&z3zE2506);
                                KILL(zTransaction)(&z3zE2507);
                                KILL(zTransaction)(&z3zE2508);
                                KILL(zTransaction)(&z3zE2509);
                                KILL(zTransaction)(&z3zE2510);
                                KILL(zTransaction)(&z3zE2511);
                                KILL(zTransaction)(&z3zE2512);
                                KILL(zTransaction)(&z3zE2513);
                                KILL(zTransaction)(&z3zE2514);
                                KILL(zTransaction)(&z3zE2515);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2625);
                                KILL(zTransaction)(&z3zE2517);
                                KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                goto end_block_exception_2965;
                              }
                            }
                            sail_u256 z2zE2636;
                            {
                              z2zE2636 = zrlp_ref_uint_word(zmp_f);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE2504);
                                KILL(zTransaction)(&z3zE2505);
                                KILL(zTransaction)(&z3zE2506);
                                KILL(zTransaction)(&z3zE2507);
                                KILL(zTransaction)(&z3zE2508);
                                KILL(zTransaction)(&z3zE2509);
                                KILL(zTransaction)(&z3zE2510);
                                KILL(zTransaction)(&z3zE2511);
                                KILL(zTransaction)(&z3zE2512);
                                KILL(zTransaction)(&z3zE2513);
                                KILL(zTransaction)(&z3zE2514);
                                KILL(zTransaction)(&z3zE2515);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2625);
                                KILL(zTransaction)(&z3zE2517);
                                KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                goto end_block_exception_2965;
                              }
                            }
                            sail_fixed_bytes_32 z2zE2638;
                            {
                              struct zByteSliceFields z2zE2637;
                              {
                                z2zE2637 = ztx_sig_span(zchain_f, zv_f);
                                if (have_exception) {
                                  KILL(zTransaction)(&z3zE2504);
                                  KILL(zTransaction)(&z3zE2505);
                                  KILL(zTransaction)(&z3zE2506);
                                  KILL(zTransaction)(&z3zE2507);
                                  KILL(zTransaction)(&z3zE2508);
                                  KILL(zTransaction)(&z3zE2509);
                                  KILL(zTransaction)(&z3zE2510);
                                  KILL(zTransaction)(&z3zE2511);
                                  KILL(zTransaction)(&z3zE2512);
                                  KILL(zTransaction)(&z3zE2513);
                                  KILL(zTransaction)(&z3zE2514);
                                  KILL(zTransaction)(&z3zE2515);
                                  KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2625);
                                  KILL(zTransaction)(&z3zE2517);
                                  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                  KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                  goto end_block_exception_2965;
                                }
                              }
                              {
                                z2zE2638 = ztx_signing_hash(zFeeMarketTx, z2zE2637, zv);
                                if (have_exception) {
                                  KILL(zTransaction)(&z3zE2504);
                                  KILL(zTransaction)(&z3zE2505);
                                  KILL(zTransaction)(&z3zE2506);
                                  KILL(zTransaction)(&z3zE2507);
                                  KILL(zTransaction)(&z3zE2508);
                                  KILL(zTransaction)(&z3zE2509);
                                  KILL(zTransaction)(&z3zE2510);
                                  KILL(zTransaction)(&z3zE2511);
                                  KILL(zTransaction)(&z3zE2512);
                                  KILL(zTransaction)(&z3zE2513);
                                  KILL(zTransaction)(&z3zE2514);
                                  KILL(zTransaction)(&z3zE2515);
                                  KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2625);
                                  KILL(zTransaction)(&z3zE2517);
                                  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                  KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                  goto end_block_exception_2965;
                                }
                              }
                            }
                            sail_u256 z2zE2639;
                            {
                              z2zE2639 = zrlp_ref_uint_word(zr_f);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE2504);
                                KILL(zTransaction)(&z3zE2505);
                                KILL(zTransaction)(&z3zE2506);
                                KILL(zTransaction)(&z3zE2507);
                                KILL(zTransaction)(&z3zE2508);
                                KILL(zTransaction)(&z3zE2509);
                                KILL(zTransaction)(&z3zE2510);
                                KILL(zTransaction)(&z3zE2511);
                                KILL(zTransaction)(&z3zE2512);
                                KILL(zTransaction)(&z3zE2513);
                                KILL(zTransaction)(&z3zE2514);
                                KILL(zTransaction)(&z3zE2515);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2625);
                                KILL(zTransaction)(&z3zE2517);
                                KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                goto end_block_exception_2965;
                              }
                            }
                            sail_u256 z2zE2640;
                            {
                              z2zE2640 = zrlp_ref_uint_word(zs_f);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE2504);
                                KILL(zTransaction)(&z3zE2505);
                                KILL(zTransaction)(&z3zE2506);
                                KILL(zTransaction)(&z3zE2507);
                                KILL(zTransaction)(&z3zE2508);
                                KILL(zTransaction)(&z3zE2509);
                                KILL(zTransaction)(&z3zE2510);
                                KILL(zTransaction)(&z3zE2511);
                                KILL(zTransaction)(&z3zE2512);
                                KILL(zTransaction)(&z3zE2513);
                                KILL(zTransaction)(&z3zE2514);
                                KILL(zTransaction)(&z3zE2515);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2625);
                                KILL(zTransaction)(&z3zE2517);
                                KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                goto end_block_exception_2965;
                              }
                            }
                            z3zE2517.zaccess_list_address_count = zal_addr_count;
                            COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE2517)->zaccess_list_addresses), zal_addrs);
                            z3zE2517.zaccess_list_slot_count = zal_slot_count;
                            COPY(zz5listz8z5structz0zzStorageKeyz9)(&((&z3zE2517)->zaccess_list_slots), zal_slots);
                            z3zE2517.zauthorizzation_count = UINT64_C(0);
                            zz5listz8z5structz0zzAuthorizzzzationz9 z3zE2519;
                            CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&z3zE2519);
                            COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&((&z3zE2517)->zauthorizzations), z3zE2519);
                            KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z3zE2519);
                            z3zE2517.zblob_hashes = zEMPTY_BLOB_HASHES;
                            z3zE2517.zchain_id = z2zE2627;
                            z3zE2517.zgas_limit = z2zE2628;
                            z3zE2517.zinput_src = z2zE2634;
                            z3zE2517.zis_create = z2zE2630;
                            z3zE2517.zmax_blob_fee = zZERO_WORD;
                            z3zE2517.zmax_fee = z2zE2635;
                            z3zE2517.zmax_priority_fee = z2zE2636;
                            z3zE2517.znonce = z2zE2626;
                            z3zE2517.zpubkey = zpubkey;
                            z3zE2517.zraw = ztx;
                            z3zE2517.zrecipient = z2zE2632;
                            z3zE2517.zsender = zsender;
                            z3zE2517.zsig_r = z2zE2639;
                            z3zE2517.zsig_s = z2zE2640;
                            z3zE2517.zsig_v = zv;
                            z3zE2517.zsigning_hash = z2zE2638;
                            z3zE2517.ztx_type = zFeeMarketTx;
                            z3zE2517.zvalue = z2zE2633;
                            KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                            KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                            goto finish_match_2962;
                          }
                        case_2963: ;
                          sail_match_failure("decode_fee_market_tx");
                        finish_match_2962: ;
                          COPY(zTransaction)(&z3zE2515, z3zE2517);
                          KILL(zTransaction)(&z3zE2517);
                          KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2625);
                          goto finish_match_2960;
                        }
                      case_2961: ;
                        sail_match_failure("decode_fee_market_tx");
                      finish_match_2960: ;
                        COPY(zTransaction)(&z3zE2514, z3zE2515);
                        KILL(zTransaction)(&z3zE2515);
                        goto finish_match_2958;
                      }
                    case_2959: ;
                      sail_match_failure("decode_fee_market_tx");
                    finish_match_2958: ;
                      COPY(zTransaction)(&z3zE2513, z3zE2514);
                      KILL(zTransaction)(&z3zE2514);
                      goto finish_match_2956;
                    }
                  case_2957: ;
                    sail_match_failure("decode_fee_market_tx");
                  finish_match_2956: ;
                    COPY(zTransaction)(&z3zE2512, z3zE2513);
                    KILL(zTransaction)(&z3zE2513);
                    goto finish_match_2954;
                  }
                case_2955: ;
                  sail_match_failure("decode_fee_market_tx");
                finish_match_2954: ;
                  COPY(zTransaction)(&z3zE2511, z3zE2512);
                  KILL(zTransaction)(&z3zE2512);
                  goto finish_match_2952;
                }
              case_2953: ;
                sail_match_failure("decode_fee_market_tx");
              finish_match_2952: ;
                COPY(zTransaction)(&z3zE2510, z3zE2511);
                KILL(zTransaction)(&z3zE2511);
                goto finish_match_2950;
              }
            case_2951: ;
              sail_match_failure("decode_fee_market_tx");
            finish_match_2950: ;
              COPY(zTransaction)(&z3zE2509, z3zE2510);
              KILL(zTransaction)(&z3zE2510);
              goto finish_match_2948;
            }
          case_2949: ;
            sail_match_failure("decode_fee_market_tx");
          finish_match_2948: ;
            COPY(zTransaction)(&z3zE2508, z3zE2509);
            KILL(zTransaction)(&z3zE2509);
            goto finish_match_2946;
          }
        case_2947: ;
          sail_match_failure("decode_fee_market_tx");
        finish_match_2946: ;
          COPY(zTransaction)(&z3zE2507, z3zE2508);
          KILL(zTransaction)(&z3zE2508);
          goto finish_match_2944;
        }
      case_2945: ;
        sail_match_failure("decode_fee_market_tx");
      finish_match_2944: ;
        COPY(zTransaction)(&z3zE2506, z3zE2507);
        KILL(zTransaction)(&z3zE2507);
        goto finish_match_2942;
      }
    case_2943: ;
      sail_match_failure("decode_fee_market_tx");
    finish_match_2942: ;
      COPY(zTransaction)(&z3zE2505, z3zE2506);
      KILL(zTransaction)(&z3zE2506);
      goto finish_match_2940;
    }
  case_2941: ;
    sail_match_failure("decode_fee_market_tx");
  finish_match_2940: ;
    COPY(zTransaction)(&z3zE2504, z3zE2505);
    KILL(zTransaction)(&z3zE2505);
    goto finish_match_2938;
  }
case_2939: ;
  sail_match_failure("decode_fee_market_tx");
finish_match_2938: ;
  COPY(zTransaction)((*(&z8zE198)), z3zE2504);
  KILL(zTransaction)(&z3zE2504);
end_function_2964: ;
  goto end_function_3645;
end_block_exception_2965: ;
  goto end_function_3645;
end_function_3645: ;
}

void zdecode_blob_tx(struct zTransaction *z8zE199, struct zByteSliceFields ztx, struct zByteSliceFields zpubkey, enum zFork zfork, sail_fixed_bytes_20 zsender, struct zByteSliceFields zfields)
{
  struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2582;
  {
    z2zE2582 = zrlp_cursor_pop(zfields);
    if (have_exception) {  goto end_block_exception_2937;  }
  }
  struct zTransaction z3zE2471;
  CREATE(zTransaction)(&z3zE2471);
  {
    struct zRlpFieldRef zchain_f;
    zchain_f = z2zE2582.ztup0;
    struct zByteSliceFields z1zE38;
    z1zE38 = z2zE2582.ztup1;
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2583;
    {
      z2zE2583 = zrlp_cursor_pop(z1zE38);
      if (have_exception) {
        KILL(zTransaction)(&z3zE2471);
        goto end_block_exception_2937;
      }
    }
    struct zTransaction z3zE2472;
    CREATE(zTransaction)(&z3zE2472);
    {
      struct zRlpFieldRef znonce_f;
      znonce_f = z2zE2583.ztup0;
      struct zByteSliceFields z1zE39;
      z1zE39 = z2zE2583.ztup1;
      struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2584;
      {
        z2zE2584 = zrlp_cursor_pop(z1zE39);
        if (have_exception) {
          KILL(zTransaction)(&z3zE2471);
          KILL(zTransaction)(&z3zE2472);
          goto end_block_exception_2937;
        }
      }
      struct zTransaction z3zE2473;
      CREATE(zTransaction)(&z3zE2473);
      {
        struct zRlpFieldRef zmp_f;
        zmp_f = z2zE2584.ztup0;
        struct zByteSliceFields z1zE40;
        z1zE40 = z2zE2584.ztup1;
        struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2585;
        {
          z2zE2585 = zrlp_cursor_pop(z1zE40);
          if (have_exception) {
            KILL(zTransaction)(&z3zE2471);
            KILL(zTransaction)(&z3zE2472);
            KILL(zTransaction)(&z3zE2473);
            goto end_block_exception_2937;
          }
        }
        struct zTransaction z3zE2474;
        CREATE(zTransaction)(&z3zE2474);
        {
          struct zRlpFieldRef zmf_f;
          zmf_f = z2zE2585.ztup0;
          struct zByteSliceFields z1zE41;
          z1zE41 = z2zE2585.ztup1;
          struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2586;
          {
            z2zE2586 = zrlp_cursor_pop(z1zE41);
            if (have_exception) {
              KILL(zTransaction)(&z3zE2471);
              KILL(zTransaction)(&z3zE2472);
              KILL(zTransaction)(&z3zE2473);
              KILL(zTransaction)(&z3zE2474);
              goto end_block_exception_2937;
            }
          }
          struct zTransaction z3zE2475;
          CREATE(zTransaction)(&z3zE2475);
          {
            struct zRlpFieldRef zgas_f;
            zgas_f = z2zE2586.ztup0;
            struct zByteSliceFields z1zE42;
            z1zE42 = z2zE2586.ztup1;
            struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2587;
            {
              z2zE2587 = zrlp_cursor_pop(z1zE42);
              if (have_exception) {
                KILL(zTransaction)(&z3zE2471);
                KILL(zTransaction)(&z3zE2472);
                KILL(zTransaction)(&z3zE2473);
                KILL(zTransaction)(&z3zE2474);
                KILL(zTransaction)(&z3zE2475);
                goto end_block_exception_2937;
              }
            }
            struct zTransaction z3zE2476;
            CREATE(zTransaction)(&z3zE2476);
            {
              struct zRlpFieldRef zto_f;
              zto_f = z2zE2587.ztup0;
              struct zByteSliceFields z1zE43;
              z1zE43 = z2zE2587.ztup1;
              struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2588;
              {
                z2zE2588 = zrlp_cursor_pop(z1zE43);
                if (have_exception) {
                  KILL(zTransaction)(&z3zE2471);
                  KILL(zTransaction)(&z3zE2472);
                  KILL(zTransaction)(&z3zE2473);
                  KILL(zTransaction)(&z3zE2474);
                  KILL(zTransaction)(&z3zE2475);
                  KILL(zTransaction)(&z3zE2476);
                  goto end_block_exception_2937;
                }
              }
              struct zTransaction z3zE2477;
              CREATE(zTransaction)(&z3zE2477);
              {
                struct zRlpFieldRef zvalue_f;
                zvalue_f = z2zE2588.ztup0;
                struct zByteSliceFields z1zE44;
                z1zE44 = z2zE2588.ztup1;
                struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2589;
                {
                  z2zE2589 = zrlp_cursor_pop(z1zE44);
                  if (have_exception) {
                    KILL(zTransaction)(&z3zE2471);
                    KILL(zTransaction)(&z3zE2472);
                    KILL(zTransaction)(&z3zE2473);
                    KILL(zTransaction)(&z3zE2474);
                    KILL(zTransaction)(&z3zE2475);
                    KILL(zTransaction)(&z3zE2476);
                    KILL(zTransaction)(&z3zE2477);
                    goto end_block_exception_2937;
                  }
                }
                struct zTransaction z3zE2478;
                CREATE(zTransaction)(&z3zE2478);
                {
                  struct zRlpFieldRef zdata_f;
                  zdata_f = z2zE2589.ztup0;
                  struct zByteSliceFields z1zE45;
                  z1zE45 = z2zE2589.ztup1;
                  struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2590;
                  {
                    z2zE2590 = zrlp_cursor_pop(z1zE45);
                    if (have_exception) {
                      KILL(zTransaction)(&z3zE2471);
                      KILL(zTransaction)(&z3zE2472);
                      KILL(zTransaction)(&z3zE2473);
                      KILL(zTransaction)(&z3zE2474);
                      KILL(zTransaction)(&z3zE2475);
                      KILL(zTransaction)(&z3zE2476);
                      KILL(zTransaction)(&z3zE2477);
                      KILL(zTransaction)(&z3zE2478);
                      goto end_block_exception_2937;
                    }
                  }
                  struct zTransaction z3zE2479;
                  CREATE(zTransaction)(&z3zE2479);
                  {
                    struct zRlpFieldRef zal_f;
                    zal_f = z2zE2590.ztup0;
                    struct zByteSliceFields z1zE46;
                    z1zE46 = z2zE2590.ztup1;
                    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2591;
                    {
                      z2zE2591 = zrlp_cursor_pop(z1zE46);
                      if (have_exception) {
                        KILL(zTransaction)(&z3zE2471);
                        KILL(zTransaction)(&z3zE2472);
                        KILL(zTransaction)(&z3zE2473);
                        KILL(zTransaction)(&z3zE2474);
                        KILL(zTransaction)(&z3zE2475);
                        KILL(zTransaction)(&z3zE2476);
                        KILL(zTransaction)(&z3zE2477);
                        KILL(zTransaction)(&z3zE2478);
                        KILL(zTransaction)(&z3zE2479);
                        goto end_block_exception_2937;
                      }
                    }
                    struct zTransaction z3zE2480;
                    CREATE(zTransaction)(&z3zE2480);
                    {
                      struct zRlpFieldRef zmbf_f;
                      zmbf_f = z2zE2591.ztup0;
                      struct zByteSliceFields z1zE47;
                      z1zE47 = z2zE2591.ztup1;
                      struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2592;
                      {
                        z2zE2592 = zrlp_cursor_pop(z1zE47);
                        if (have_exception) {
                          KILL(zTransaction)(&z3zE2471);
                          KILL(zTransaction)(&z3zE2472);
                          KILL(zTransaction)(&z3zE2473);
                          KILL(zTransaction)(&z3zE2474);
                          KILL(zTransaction)(&z3zE2475);
                          KILL(zTransaction)(&z3zE2476);
                          KILL(zTransaction)(&z3zE2477);
                          KILL(zTransaction)(&z3zE2478);
                          KILL(zTransaction)(&z3zE2479);
                          KILL(zTransaction)(&z3zE2480);
                          goto end_block_exception_2937;
                        }
                      }
                      struct zTransaction z3zE2481;
                      CREATE(zTransaction)(&z3zE2481);
                      {
                        struct zRlpFieldRef zbh_f;
                        zbh_f = z2zE2592.ztup0;
                        struct zByteSliceFields z1zE48;
                        z1zE48 = z2zE2592.ztup1;
                        struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2593;
                        {
                          z2zE2593 = zrlp_cursor_pop(z1zE48);
                          if (have_exception) {
                            KILL(zTransaction)(&z3zE2471);
                            KILL(zTransaction)(&z3zE2472);
                            KILL(zTransaction)(&z3zE2473);
                            KILL(zTransaction)(&z3zE2474);
                            KILL(zTransaction)(&z3zE2475);
                            KILL(zTransaction)(&z3zE2476);
                            KILL(zTransaction)(&z3zE2477);
                            KILL(zTransaction)(&z3zE2478);
                            KILL(zTransaction)(&z3zE2479);
                            KILL(zTransaction)(&z3zE2480);
                            KILL(zTransaction)(&z3zE2481);
                            goto end_block_exception_2937;
                          }
                        }
                        struct zTransaction z3zE2482;
                        CREATE(zTransaction)(&z3zE2482);
                        {
                          struct zRlpFieldRef zv_f;
                          zv_f = z2zE2593.ztup0;
                          struct zByteSliceFields z1zE49;
                          z1zE49 = z2zE2593.ztup1;
                          struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2594;
                          {
                            z2zE2594 = zrlp_cursor_pop(z1zE49);
                            if (have_exception) {
                              KILL(zTransaction)(&z3zE2471);
                              KILL(zTransaction)(&z3zE2472);
                              KILL(zTransaction)(&z3zE2473);
                              KILL(zTransaction)(&z3zE2474);
                              KILL(zTransaction)(&z3zE2475);
                              KILL(zTransaction)(&z3zE2476);
                              KILL(zTransaction)(&z3zE2477);
                              KILL(zTransaction)(&z3zE2478);
                              KILL(zTransaction)(&z3zE2479);
                              KILL(zTransaction)(&z3zE2480);
                              KILL(zTransaction)(&z3zE2481);
                              KILL(zTransaction)(&z3zE2482);
                              goto end_block_exception_2937;
                            }
                          }
                          struct zTransaction z3zE2483;
                          CREATE(zTransaction)(&z3zE2483);
                          {
                            struct zRlpFieldRef zr_f;
                            zr_f = z2zE2594.ztup0;
                            struct zByteSliceFields z1zE50;
                            z1zE50 = z2zE2594.ztup1;
                            struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2595;
                            {
                              z2zE2595 = zrlp_cursor_pop(z1zE50);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE2471);
                                KILL(zTransaction)(&z3zE2472);
                                KILL(zTransaction)(&z3zE2473);
                                KILL(zTransaction)(&z3zE2474);
                                KILL(zTransaction)(&z3zE2475);
                                KILL(zTransaction)(&z3zE2476);
                                KILL(zTransaction)(&z3zE2477);
                                KILL(zTransaction)(&z3zE2478);
                                KILL(zTransaction)(&z3zE2479);
                                KILL(zTransaction)(&z3zE2480);
                                KILL(zTransaction)(&z3zE2481);
                                KILL(zTransaction)(&z3zE2482);
                                KILL(zTransaction)(&z3zE2483);
                                goto end_block_exception_2937;
                              }
                            }
                            struct zTransaction z3zE2484;
                            CREATE(zTransaction)(&z3zE2484);
                            {
                              struct zRlpFieldRef zs_f;
                              zs_f = z2zE2595.ztup0;
                              struct zByteSliceFields z1zE51;
                              z1zE51 = z2zE2595.ztup1;
                              unit z3zE2485;
                              {
                                z3zE2485 = zrlp_cursor_expect_end(z1zE51);
                                if (have_exception) {
                                  KILL(zTransaction)(&z3zE2471);
                                  KILL(zTransaction)(&z3zE2472);
                                  KILL(zTransaction)(&z3zE2473);
                                  KILL(zTransaction)(&z3zE2474);
                                  KILL(zTransaction)(&z3zE2475);
                                  KILL(zTransaction)(&z3zE2476);
                                  KILL(zTransaction)(&z3zE2477);
                                  KILL(zTransaction)(&z3zE2478);
                                  KILL(zTransaction)(&z3zE2479);
                                  KILL(zTransaction)(&z3zE2480);
                                  KILL(zTransaction)(&z3zE2481);
                                  KILL(zTransaction)(&z3zE2482);
                                  KILL(zTransaction)(&z3zE2483);
                                  KILL(zTransaction)(&z3zE2484);
                                  goto end_block_exception_2937;
                                }
                              }
                              sail_u256 zv;
                              {
                                zv = zrlp_ref_word(zv_f);
                                if (have_exception) {
                                  KILL(zTransaction)(&z3zE2471);
                                  KILL(zTransaction)(&z3zE2472);
                                  KILL(zTransaction)(&z3zE2473);
                                  KILL(zTransaction)(&z3zE2474);
                                  KILL(zTransaction)(&z3zE2475);
                                  KILL(zTransaction)(&z3zE2476);
                                  KILL(zTransaction)(&z3zE2477);
                                  KILL(zTransaction)(&z3zE2478);
                                  KILL(zTransaction)(&z3zE2479);
                                  KILL(zTransaction)(&z3zE2480);
                                  KILL(zTransaction)(&z3zE2481);
                                  KILL(zTransaction)(&z3zE2482);
                                  KILL(zTransaction)(&z3zE2483);
                                  KILL(zTransaction)(&z3zE2484);
                                  goto end_block_exception_2937;
                                }
                              }
                              struct ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9 z2zE2596;
                              CREATE(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2596);
                              {
                                zdecode_access_list(&z2zE2596, zal_f);
                                if (have_exception) {
                                  KILL(zTransaction)(&z3zE2471);
                                  KILL(zTransaction)(&z3zE2472);
                                  KILL(zTransaction)(&z3zE2473);
                                  KILL(zTransaction)(&z3zE2474);
                                  KILL(zTransaction)(&z3zE2475);
                                  KILL(zTransaction)(&z3zE2476);
                                  KILL(zTransaction)(&z3zE2477);
                                  KILL(zTransaction)(&z3zE2478);
                                  KILL(zTransaction)(&z3zE2479);
                                  KILL(zTransaction)(&z3zE2480);
                                  KILL(zTransaction)(&z3zE2481);
                                  KILL(zTransaction)(&z3zE2482);
                                  KILL(zTransaction)(&z3zE2483);
                                  KILL(zTransaction)(&z3zE2484);
                                  KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2596);
                                  goto end_block_exception_2937;
                                }
                              }
                              struct zTransaction z3zE2486;
                              CREATE(zTransaction)(&z3zE2486);
                              {
                                zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 zal_addrs;
                                CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs, z2zE2596.ztup0);
                                zz5listz8z5structz0zzStorageKeyz9 zal_slots;
                                CREATE(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                COPY(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots, z2zE2596.ztup1);
                                uint64_t zal_addr_count;
                                zal_addr_count = z2zE2596.ztup2;
                                uint64_t zal_slot_count;
                                zal_slot_count = z2zE2596.ztup3;
                                struct zBlobHashes zblob_hashes;
                                {
                                  zblob_hashes = zdecode_blob_hashes(zbh_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE2471);
                                    KILL(zTransaction)(&z3zE2472);
                                    KILL(zTransaction)(&z3zE2473);
                                    KILL(zTransaction)(&z3zE2474);
                                    KILL(zTransaction)(&z3zE2475);
                                    KILL(zTransaction)(&z3zE2476);
                                    KILL(zTransaction)(&z3zE2477);
                                    KILL(zTransaction)(&z3zE2478);
                                    KILL(zTransaction)(&z3zE2479);
                                    KILL(zTransaction)(&z3zE2480);
                                    KILL(zTransaction)(&z3zE2481);
                                    KILL(zTransaction)(&z3zE2482);
                                    KILL(zTransaction)(&z3zE2483);
                                    KILL(zTransaction)(&z3zE2484);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2596);
                                    KILL(zTransaction)(&z3zE2486);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    goto end_block_exception_2937;
                                  }
                                }
                                sail_u256 z2zE2597;
                                {
                                  z2zE2597 = zrlp_ref_uint_word(znonce_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE2471);
                                    KILL(zTransaction)(&z3zE2472);
                                    KILL(zTransaction)(&z3zE2473);
                                    KILL(zTransaction)(&z3zE2474);
                                    KILL(zTransaction)(&z3zE2475);
                                    KILL(zTransaction)(&z3zE2476);
                                    KILL(zTransaction)(&z3zE2477);
                                    KILL(zTransaction)(&z3zE2478);
                                    KILL(zTransaction)(&z3zE2479);
                                    KILL(zTransaction)(&z3zE2480);
                                    KILL(zTransaction)(&z3zE2481);
                                    KILL(zTransaction)(&z3zE2482);
                                    KILL(zTransaction)(&z3zE2483);
                                    KILL(zTransaction)(&z3zE2484);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2596);
                                    KILL(zTransaction)(&z3zE2486);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    goto end_block_exception_2937;
                                  }
                                }
                                uint64_t z2zE2598;
                                {
                                  z2zE2598 = zrlp_ref_uint64(zchain_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE2471);
                                    KILL(zTransaction)(&z3zE2472);
                                    KILL(zTransaction)(&z3zE2473);
                                    KILL(zTransaction)(&z3zE2474);
                                    KILL(zTransaction)(&z3zE2475);
                                    KILL(zTransaction)(&z3zE2476);
                                    KILL(zTransaction)(&z3zE2477);
                                    KILL(zTransaction)(&z3zE2478);
                                    KILL(zTransaction)(&z3zE2479);
                                    KILL(zTransaction)(&z3zE2480);
                                    KILL(zTransaction)(&z3zE2481);
                                    KILL(zTransaction)(&z3zE2482);
                                    KILL(zTransaction)(&z3zE2483);
                                    KILL(zTransaction)(&z3zE2484);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2596);
                                    KILL(zTransaction)(&z3zE2486);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    goto end_block_exception_2937;
                                  }
                                }
                                uint64_t z2zE2599;
                                {
                                  z2zE2599 = zrlp_ref_gas(zgas_f, zfork);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE2471);
                                    KILL(zTransaction)(&z3zE2472);
                                    KILL(zTransaction)(&z3zE2473);
                                    KILL(zTransaction)(&z3zE2474);
                                    KILL(zTransaction)(&z3zE2475);
                                    KILL(zTransaction)(&z3zE2476);
                                    KILL(zTransaction)(&z3zE2477);
                                    KILL(zTransaction)(&z3zE2478);
                                    KILL(zTransaction)(&z3zE2479);
                                    KILL(zTransaction)(&z3zE2480);
                                    KILL(zTransaction)(&z3zE2481);
                                    KILL(zTransaction)(&z3zE2482);
                                    KILL(zTransaction)(&z3zE2483);
                                    KILL(zTransaction)(&z3zE2484);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2596);
                                    KILL(zTransaction)(&z3zE2486);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    goto end_block_exception_2937;
                                  }
                                }
                                bool z2zE2601;
                                {
                                  uint64_t z2zE2600;
                                  z2zE2600 = zto_f.zcontent_len;
                                  z2zE2601 = (z2zE2600 == UINT64_C(0));
                                }
                                sail_fixed_bytes_20 z2zE2603;
                                {
                                  sail_u256 z2zE2602;
                                  {
                                    z2zE2602 = zrlp_ref_word(zto_f);
                                    if (have_exception) {
                                      KILL(zTransaction)(&z3zE2471);
                                      KILL(zTransaction)(&z3zE2472);
                                      KILL(zTransaction)(&z3zE2473);
                                      KILL(zTransaction)(&z3zE2474);
                                      KILL(zTransaction)(&z3zE2475);
                                      KILL(zTransaction)(&z3zE2476);
                                      KILL(zTransaction)(&z3zE2477);
                                      KILL(zTransaction)(&z3zE2478);
                                      KILL(zTransaction)(&z3zE2479);
                                      KILL(zTransaction)(&z3zE2480);
                                      KILL(zTransaction)(&z3zE2481);
                                      KILL(zTransaction)(&z3zE2482);
                                      KILL(zTransaction)(&z3zE2483);
                                      KILL(zTransaction)(&z3zE2484);
                                      KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2596);
                                      KILL(zTransaction)(&z3zE2486);
                                      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                      KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                      goto end_block_exception_2937;
                                    }
                                  }
                                  z2zE2603 = evmsail_word_to_address(z2zE2602);
                                }
                                sail_u256 z2zE2604;
                                {
                                  z2zE2604 = zrlp_ref_uint_word(zvalue_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE2471);
                                    KILL(zTransaction)(&z3zE2472);
                                    KILL(zTransaction)(&z3zE2473);
                                    KILL(zTransaction)(&z3zE2474);
                                    KILL(zTransaction)(&z3zE2475);
                                    KILL(zTransaction)(&z3zE2476);
                                    KILL(zTransaction)(&z3zE2477);
                                    KILL(zTransaction)(&z3zE2478);
                                    KILL(zTransaction)(&z3zE2479);
                                    KILL(zTransaction)(&z3zE2480);
                                    KILL(zTransaction)(&z3zE2481);
                                    KILL(zTransaction)(&z3zE2482);
                                    KILL(zTransaction)(&z3zE2483);
                                    KILL(zTransaction)(&z3zE2484);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2596);
                                    KILL(zTransaction)(&z3zE2486);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    goto end_block_exception_2937;
                                  }
                                }
                                struct zByteSliceFields z2zE2605;
                                {
                                  z2zE2605 = ztx_input_span(zdata_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE2471);
                                    KILL(zTransaction)(&z3zE2472);
                                    KILL(zTransaction)(&z3zE2473);
                                    KILL(zTransaction)(&z3zE2474);
                                    KILL(zTransaction)(&z3zE2475);
                                    KILL(zTransaction)(&z3zE2476);
                                    KILL(zTransaction)(&z3zE2477);
                                    KILL(zTransaction)(&z3zE2478);
                                    KILL(zTransaction)(&z3zE2479);
                                    KILL(zTransaction)(&z3zE2480);
                                    KILL(zTransaction)(&z3zE2481);
                                    KILL(zTransaction)(&z3zE2482);
                                    KILL(zTransaction)(&z3zE2483);
                                    KILL(zTransaction)(&z3zE2484);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2596);
                                    KILL(zTransaction)(&z3zE2486);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    goto end_block_exception_2937;
                                  }
                                }
                                sail_u256 z2zE2606;
                                {
                                  z2zE2606 = zrlp_ref_uint_word(zmf_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE2471);
                                    KILL(zTransaction)(&z3zE2472);
                                    KILL(zTransaction)(&z3zE2473);
                                    KILL(zTransaction)(&z3zE2474);
                                    KILL(zTransaction)(&z3zE2475);
                                    KILL(zTransaction)(&z3zE2476);
                                    KILL(zTransaction)(&z3zE2477);
                                    KILL(zTransaction)(&z3zE2478);
                                    KILL(zTransaction)(&z3zE2479);
                                    KILL(zTransaction)(&z3zE2480);
                                    KILL(zTransaction)(&z3zE2481);
                                    KILL(zTransaction)(&z3zE2482);
                                    KILL(zTransaction)(&z3zE2483);
                                    KILL(zTransaction)(&z3zE2484);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2596);
                                    KILL(zTransaction)(&z3zE2486);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    goto end_block_exception_2937;
                                  }
                                }
                                sail_u256 z2zE2607;
                                {
                                  z2zE2607 = zrlp_ref_uint_word(zmbf_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE2471);
                                    KILL(zTransaction)(&z3zE2472);
                                    KILL(zTransaction)(&z3zE2473);
                                    KILL(zTransaction)(&z3zE2474);
                                    KILL(zTransaction)(&z3zE2475);
                                    KILL(zTransaction)(&z3zE2476);
                                    KILL(zTransaction)(&z3zE2477);
                                    KILL(zTransaction)(&z3zE2478);
                                    KILL(zTransaction)(&z3zE2479);
                                    KILL(zTransaction)(&z3zE2480);
                                    KILL(zTransaction)(&z3zE2481);
                                    KILL(zTransaction)(&z3zE2482);
                                    KILL(zTransaction)(&z3zE2483);
                                    KILL(zTransaction)(&z3zE2484);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2596);
                                    KILL(zTransaction)(&z3zE2486);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    goto end_block_exception_2937;
                                  }
                                }
                                sail_u256 z2zE2608;
                                {
                                  z2zE2608 = zrlp_ref_uint_word(zmp_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE2471);
                                    KILL(zTransaction)(&z3zE2472);
                                    KILL(zTransaction)(&z3zE2473);
                                    KILL(zTransaction)(&z3zE2474);
                                    KILL(zTransaction)(&z3zE2475);
                                    KILL(zTransaction)(&z3zE2476);
                                    KILL(zTransaction)(&z3zE2477);
                                    KILL(zTransaction)(&z3zE2478);
                                    KILL(zTransaction)(&z3zE2479);
                                    KILL(zTransaction)(&z3zE2480);
                                    KILL(zTransaction)(&z3zE2481);
                                    KILL(zTransaction)(&z3zE2482);
                                    KILL(zTransaction)(&z3zE2483);
                                    KILL(zTransaction)(&z3zE2484);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2596);
                                    KILL(zTransaction)(&z3zE2486);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    goto end_block_exception_2937;
                                  }
                                }
                                sail_fixed_bytes_32 z2zE2610;
                                {
                                  struct zByteSliceFields z2zE2609;
                                  {
                                    z2zE2609 = ztx_sig_span(zchain_f, zv_f);
                                    if (have_exception) {
                                      KILL(zTransaction)(&z3zE2471);
                                      KILL(zTransaction)(&z3zE2472);
                                      KILL(zTransaction)(&z3zE2473);
                                      KILL(zTransaction)(&z3zE2474);
                                      KILL(zTransaction)(&z3zE2475);
                                      KILL(zTransaction)(&z3zE2476);
                                      KILL(zTransaction)(&z3zE2477);
                                      KILL(zTransaction)(&z3zE2478);
                                      KILL(zTransaction)(&z3zE2479);
                                      KILL(zTransaction)(&z3zE2480);
                                      KILL(zTransaction)(&z3zE2481);
                                      KILL(zTransaction)(&z3zE2482);
                                      KILL(zTransaction)(&z3zE2483);
                                      KILL(zTransaction)(&z3zE2484);
                                      KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2596);
                                      KILL(zTransaction)(&z3zE2486);
                                      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                      KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                      goto end_block_exception_2937;
                                    }
                                  }
                                  {
                                    z2zE2610 = ztx_signing_hash(zBlobTx, z2zE2609, zv);
                                    if (have_exception) {
                                      KILL(zTransaction)(&z3zE2471);
                                      KILL(zTransaction)(&z3zE2472);
                                      KILL(zTransaction)(&z3zE2473);
                                      KILL(zTransaction)(&z3zE2474);
                                      KILL(zTransaction)(&z3zE2475);
                                      KILL(zTransaction)(&z3zE2476);
                                      KILL(zTransaction)(&z3zE2477);
                                      KILL(zTransaction)(&z3zE2478);
                                      KILL(zTransaction)(&z3zE2479);
                                      KILL(zTransaction)(&z3zE2480);
                                      KILL(zTransaction)(&z3zE2481);
                                      KILL(zTransaction)(&z3zE2482);
                                      KILL(zTransaction)(&z3zE2483);
                                      KILL(zTransaction)(&z3zE2484);
                                      KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2596);
                                      KILL(zTransaction)(&z3zE2486);
                                      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                      KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                      goto end_block_exception_2937;
                                    }
                                  }
                                }
                                sail_u256 z2zE2611;
                                {
                                  z2zE2611 = zrlp_ref_uint_word(zr_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE2471);
                                    KILL(zTransaction)(&z3zE2472);
                                    KILL(zTransaction)(&z3zE2473);
                                    KILL(zTransaction)(&z3zE2474);
                                    KILL(zTransaction)(&z3zE2475);
                                    KILL(zTransaction)(&z3zE2476);
                                    KILL(zTransaction)(&z3zE2477);
                                    KILL(zTransaction)(&z3zE2478);
                                    KILL(zTransaction)(&z3zE2479);
                                    KILL(zTransaction)(&z3zE2480);
                                    KILL(zTransaction)(&z3zE2481);
                                    KILL(zTransaction)(&z3zE2482);
                                    KILL(zTransaction)(&z3zE2483);
                                    KILL(zTransaction)(&z3zE2484);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2596);
                                    KILL(zTransaction)(&z3zE2486);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    goto end_block_exception_2937;
                                  }
                                }
                                sail_u256 z2zE2612;
                                {
                                  z2zE2612 = zrlp_ref_uint_word(zs_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE2471);
                                    KILL(zTransaction)(&z3zE2472);
                                    KILL(zTransaction)(&z3zE2473);
                                    KILL(zTransaction)(&z3zE2474);
                                    KILL(zTransaction)(&z3zE2475);
                                    KILL(zTransaction)(&z3zE2476);
                                    KILL(zTransaction)(&z3zE2477);
                                    KILL(zTransaction)(&z3zE2478);
                                    KILL(zTransaction)(&z3zE2479);
                                    KILL(zTransaction)(&z3zE2480);
                                    KILL(zTransaction)(&z3zE2481);
                                    KILL(zTransaction)(&z3zE2482);
                                    KILL(zTransaction)(&z3zE2483);
                                    KILL(zTransaction)(&z3zE2484);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2596);
                                    KILL(zTransaction)(&z3zE2486);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    goto end_block_exception_2937;
                                  }
                                }
                                z3zE2486.zaccess_list_address_count = zal_addr_count;
                                COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE2486)->zaccess_list_addresses), zal_addrs);
                                z3zE2486.zaccess_list_slot_count = zal_slot_count;
                                COPY(zz5listz8z5structz0zzStorageKeyz9)(&((&z3zE2486)->zaccess_list_slots), zal_slots);
                                z3zE2486.zauthorizzation_count = UINT64_C(0);
                                zz5listz8z5structz0zzAuthorizzzzationz9 z3zE2488;
                                CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&z3zE2488);
                                COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&((&z3zE2486)->zauthorizzations), z3zE2488);
                                KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z3zE2488);
                                z3zE2486.zblob_hashes = zblob_hashes;
                                z3zE2486.zchain_id = z2zE2598;
                                z3zE2486.zgas_limit = z2zE2599;
                                z3zE2486.zinput_src = z2zE2605;
                                z3zE2486.zis_create = z2zE2601;
                                z3zE2486.zmax_blob_fee = z2zE2607;
                                z3zE2486.zmax_fee = z2zE2606;
                                z3zE2486.zmax_priority_fee = z2zE2608;
                                z3zE2486.znonce = z2zE2597;
                                z3zE2486.zpubkey = zpubkey;
                                z3zE2486.zraw = ztx;
                                z3zE2486.zrecipient = z2zE2603;
                                z3zE2486.zsender = zsender;
                                z3zE2486.zsig_r = z2zE2611;
                                z3zE2486.zsig_s = z2zE2612;
                                z3zE2486.zsig_v = zv;
                                z3zE2486.zsigning_hash = z2zE2610;
                                z3zE2486.ztx_type = zBlobTx;
                                z3zE2486.zvalue = z2zE2604;
                                KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                goto finish_match_2934;
                              }
                            case_2935: ;
                              sail_match_failure("decode_blob_tx");
                            finish_match_2934: ;
                              COPY(zTransaction)(&z3zE2484, z3zE2486);
                              KILL(zTransaction)(&z3zE2486);
                              KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2596);
                              goto finish_match_2932;
                            }
                          case_2933: ;
                            sail_match_failure("decode_blob_tx");
                          finish_match_2932: ;
                            COPY(zTransaction)(&z3zE2483, z3zE2484);
                            KILL(zTransaction)(&z3zE2484);
                            goto finish_match_2930;
                          }
                        case_2931: ;
                          sail_match_failure("decode_blob_tx");
                        finish_match_2930: ;
                          COPY(zTransaction)(&z3zE2482, z3zE2483);
                          KILL(zTransaction)(&z3zE2483);
                          goto finish_match_2928;
                        }
                      case_2929: ;
                        sail_match_failure("decode_blob_tx");
                      finish_match_2928: ;
                        COPY(zTransaction)(&z3zE2481, z3zE2482);
                        KILL(zTransaction)(&z3zE2482);
                        goto finish_match_2926;
                      }
                    case_2927: ;
                      sail_match_failure("decode_blob_tx");
                    finish_match_2926: ;
                      COPY(zTransaction)(&z3zE2480, z3zE2481);
                      KILL(zTransaction)(&z3zE2481);
                      goto finish_match_2924;
                    }
                  case_2925: ;
                    sail_match_failure("decode_blob_tx");
                  finish_match_2924: ;
                    COPY(zTransaction)(&z3zE2479, z3zE2480);
                    KILL(zTransaction)(&z3zE2480);
                    goto finish_match_2922;
                  }
                case_2923: ;
                  sail_match_failure("decode_blob_tx");
                finish_match_2922: ;
                  COPY(zTransaction)(&z3zE2478, z3zE2479);
                  KILL(zTransaction)(&z3zE2479);
                  goto finish_match_2920;
                }
              case_2921: ;
                sail_match_failure("decode_blob_tx");
              finish_match_2920: ;
                COPY(zTransaction)(&z3zE2477, z3zE2478);
                KILL(zTransaction)(&z3zE2478);
                goto finish_match_2918;
              }
            case_2919: ;
              sail_match_failure("decode_blob_tx");
            finish_match_2918: ;
              COPY(zTransaction)(&z3zE2476, z3zE2477);
              KILL(zTransaction)(&z3zE2477);
              goto finish_match_2916;
            }
          case_2917: ;
            sail_match_failure("decode_blob_tx");
          finish_match_2916: ;
            COPY(zTransaction)(&z3zE2475, z3zE2476);
            KILL(zTransaction)(&z3zE2476);
            goto finish_match_2914;
          }
        case_2915: ;
          sail_match_failure("decode_blob_tx");
        finish_match_2914: ;
          COPY(zTransaction)(&z3zE2474, z3zE2475);
          KILL(zTransaction)(&z3zE2475);
          goto finish_match_2912;
        }
      case_2913: ;
        sail_match_failure("decode_blob_tx");
      finish_match_2912: ;
        COPY(zTransaction)(&z3zE2473, z3zE2474);
        KILL(zTransaction)(&z3zE2474);
        goto finish_match_2910;
      }
    case_2911: ;
      sail_match_failure("decode_blob_tx");
    finish_match_2910: ;
      COPY(zTransaction)(&z3zE2472, z3zE2473);
      KILL(zTransaction)(&z3zE2473);
      goto finish_match_2908;
    }
  case_2909: ;
    sail_match_failure("decode_blob_tx");
  finish_match_2908: ;
    COPY(zTransaction)(&z3zE2471, z3zE2472);
    KILL(zTransaction)(&z3zE2472);
    goto finish_match_2906;
  }
case_2907: ;
  sail_match_failure("decode_blob_tx");
finish_match_2906: ;
  COPY(zTransaction)((*(&z8zE199)), z3zE2471);
  KILL(zTransaction)(&z3zE2471);
end_function_2936: ;
  goto end_function_3644;
end_block_exception_2937: ;
  goto end_function_3644;
end_function_3644: ;
}

void zdecode_set_code_tx(struct zTransaction *z8zE200, struct zByteSliceFields ztx, struct zByteSliceFields zpubkey, enum zFork zfork, sail_fixed_bytes_20 zsender, struct zByteSliceFields zfields)
{
  struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2551;
  {
    z2zE2551 = zrlp_cursor_pop(zfields);
    if (have_exception) {  goto end_block_exception_2905;  }
  }
  struct zTransaction z3zE2439;
  CREATE(zTransaction)(&z3zE2439);
  {
    struct zRlpFieldRef zchain_f;
    zchain_f = z2zE2551.ztup0;
    struct zByteSliceFields z1zE25;
    z1zE25 = z2zE2551.ztup1;
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2552;
    {
      z2zE2552 = zrlp_cursor_pop(z1zE25);
      if (have_exception) {
        KILL(zTransaction)(&z3zE2439);
        goto end_block_exception_2905;
      }
    }
    struct zTransaction z3zE2440;
    CREATE(zTransaction)(&z3zE2440);
    {
      struct zRlpFieldRef znonce_f;
      znonce_f = z2zE2552.ztup0;
      struct zByteSliceFields z1zE26;
      z1zE26 = z2zE2552.ztup1;
      struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2553;
      {
        z2zE2553 = zrlp_cursor_pop(z1zE26);
        if (have_exception) {
          KILL(zTransaction)(&z3zE2439);
          KILL(zTransaction)(&z3zE2440);
          goto end_block_exception_2905;
        }
      }
      struct zTransaction z3zE2441;
      CREATE(zTransaction)(&z3zE2441);
      {
        struct zRlpFieldRef zmp_f;
        zmp_f = z2zE2553.ztup0;
        struct zByteSliceFields z1zE27;
        z1zE27 = z2zE2553.ztup1;
        struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2554;
        {
          z2zE2554 = zrlp_cursor_pop(z1zE27);
          if (have_exception) {
            KILL(zTransaction)(&z3zE2439);
            KILL(zTransaction)(&z3zE2440);
            KILL(zTransaction)(&z3zE2441);
            goto end_block_exception_2905;
          }
        }
        struct zTransaction z3zE2442;
        CREATE(zTransaction)(&z3zE2442);
        {
          struct zRlpFieldRef zmf_f;
          zmf_f = z2zE2554.ztup0;
          struct zByteSliceFields z1zE28;
          z1zE28 = z2zE2554.ztup1;
          struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2555;
          {
            z2zE2555 = zrlp_cursor_pop(z1zE28);
            if (have_exception) {
              KILL(zTransaction)(&z3zE2439);
              KILL(zTransaction)(&z3zE2440);
              KILL(zTransaction)(&z3zE2441);
              KILL(zTransaction)(&z3zE2442);
              goto end_block_exception_2905;
            }
          }
          struct zTransaction z3zE2443;
          CREATE(zTransaction)(&z3zE2443);
          {
            struct zRlpFieldRef zgas_f;
            zgas_f = z2zE2555.ztup0;
            struct zByteSliceFields z1zE29;
            z1zE29 = z2zE2555.ztup1;
            struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2556;
            {
              z2zE2556 = zrlp_cursor_pop(z1zE29);
              if (have_exception) {
                KILL(zTransaction)(&z3zE2439);
                KILL(zTransaction)(&z3zE2440);
                KILL(zTransaction)(&z3zE2441);
                KILL(zTransaction)(&z3zE2442);
                KILL(zTransaction)(&z3zE2443);
                goto end_block_exception_2905;
              }
            }
            struct zTransaction z3zE2444;
            CREATE(zTransaction)(&z3zE2444);
            {
              struct zRlpFieldRef zto_f;
              zto_f = z2zE2556.ztup0;
              struct zByteSliceFields z1zE30;
              z1zE30 = z2zE2556.ztup1;
              struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2557;
              {
                z2zE2557 = zrlp_cursor_pop(z1zE30);
                if (have_exception) {
                  KILL(zTransaction)(&z3zE2439);
                  KILL(zTransaction)(&z3zE2440);
                  KILL(zTransaction)(&z3zE2441);
                  KILL(zTransaction)(&z3zE2442);
                  KILL(zTransaction)(&z3zE2443);
                  KILL(zTransaction)(&z3zE2444);
                  goto end_block_exception_2905;
                }
              }
              struct zTransaction z3zE2445;
              CREATE(zTransaction)(&z3zE2445);
              {
                struct zRlpFieldRef zvalue_f;
                zvalue_f = z2zE2557.ztup0;
                struct zByteSliceFields z1zE31;
                z1zE31 = z2zE2557.ztup1;
                struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2558;
                {
                  z2zE2558 = zrlp_cursor_pop(z1zE31);
                  if (have_exception) {
                    KILL(zTransaction)(&z3zE2439);
                    KILL(zTransaction)(&z3zE2440);
                    KILL(zTransaction)(&z3zE2441);
                    KILL(zTransaction)(&z3zE2442);
                    KILL(zTransaction)(&z3zE2443);
                    KILL(zTransaction)(&z3zE2444);
                    KILL(zTransaction)(&z3zE2445);
                    goto end_block_exception_2905;
                  }
                }
                struct zTransaction z3zE2446;
                CREATE(zTransaction)(&z3zE2446);
                {
                  struct zRlpFieldRef zdata_f;
                  zdata_f = z2zE2558.ztup0;
                  struct zByteSliceFields z1zE32;
                  z1zE32 = z2zE2558.ztup1;
                  struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2559;
                  {
                    z2zE2559 = zrlp_cursor_pop(z1zE32);
                    if (have_exception) {
                      KILL(zTransaction)(&z3zE2439);
                      KILL(zTransaction)(&z3zE2440);
                      KILL(zTransaction)(&z3zE2441);
                      KILL(zTransaction)(&z3zE2442);
                      KILL(zTransaction)(&z3zE2443);
                      KILL(zTransaction)(&z3zE2444);
                      KILL(zTransaction)(&z3zE2445);
                      KILL(zTransaction)(&z3zE2446);
                      goto end_block_exception_2905;
                    }
                  }
                  struct zTransaction z3zE2447;
                  CREATE(zTransaction)(&z3zE2447);
                  {
                    struct zRlpFieldRef zal_f;
                    zal_f = z2zE2559.ztup0;
                    struct zByteSliceFields z1zE33;
                    z1zE33 = z2zE2559.ztup1;
                    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2560;
                    {
                      z2zE2560 = zrlp_cursor_pop(z1zE33);
                      if (have_exception) {
                        KILL(zTransaction)(&z3zE2439);
                        KILL(zTransaction)(&z3zE2440);
                        KILL(zTransaction)(&z3zE2441);
                        KILL(zTransaction)(&z3zE2442);
                        KILL(zTransaction)(&z3zE2443);
                        KILL(zTransaction)(&z3zE2444);
                        KILL(zTransaction)(&z3zE2445);
                        KILL(zTransaction)(&z3zE2446);
                        KILL(zTransaction)(&z3zE2447);
                        goto end_block_exception_2905;
                      }
                    }
                    struct zTransaction z3zE2448;
                    CREATE(zTransaction)(&z3zE2448);
                    {
                      struct zRlpFieldRef zauth_f;
                      zauth_f = z2zE2560.ztup0;
                      struct zByteSliceFields z1zE34;
                      z1zE34 = z2zE2560.ztup1;
                      struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2561;
                      {
                        z2zE2561 = zrlp_cursor_pop(z1zE34);
                        if (have_exception) {
                          KILL(zTransaction)(&z3zE2439);
                          KILL(zTransaction)(&z3zE2440);
                          KILL(zTransaction)(&z3zE2441);
                          KILL(zTransaction)(&z3zE2442);
                          KILL(zTransaction)(&z3zE2443);
                          KILL(zTransaction)(&z3zE2444);
                          KILL(zTransaction)(&z3zE2445);
                          KILL(zTransaction)(&z3zE2446);
                          KILL(zTransaction)(&z3zE2447);
                          KILL(zTransaction)(&z3zE2448);
                          goto end_block_exception_2905;
                        }
                      }
                      struct zTransaction z3zE2449;
                      CREATE(zTransaction)(&z3zE2449);
                      {
                        struct zRlpFieldRef zv_f;
                        zv_f = z2zE2561.ztup0;
                        struct zByteSliceFields z1zE35;
                        z1zE35 = z2zE2561.ztup1;
                        struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2562;
                        {
                          z2zE2562 = zrlp_cursor_pop(z1zE35);
                          if (have_exception) {
                            KILL(zTransaction)(&z3zE2439);
                            KILL(zTransaction)(&z3zE2440);
                            KILL(zTransaction)(&z3zE2441);
                            KILL(zTransaction)(&z3zE2442);
                            KILL(zTransaction)(&z3zE2443);
                            KILL(zTransaction)(&z3zE2444);
                            KILL(zTransaction)(&z3zE2445);
                            KILL(zTransaction)(&z3zE2446);
                            KILL(zTransaction)(&z3zE2447);
                            KILL(zTransaction)(&z3zE2448);
                            KILL(zTransaction)(&z3zE2449);
                            goto end_block_exception_2905;
                          }
                        }
                        struct zTransaction z3zE2450;
                        CREATE(zTransaction)(&z3zE2450);
                        {
                          struct zRlpFieldRef zr_f;
                          zr_f = z2zE2562.ztup0;
                          struct zByteSliceFields z1zE36;
                          z1zE36 = z2zE2562.ztup1;
                          struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE2563;
                          {
                            z2zE2563 = zrlp_cursor_pop(z1zE36);
                            if (have_exception) {
                              KILL(zTransaction)(&z3zE2439);
                              KILL(zTransaction)(&z3zE2440);
                              KILL(zTransaction)(&z3zE2441);
                              KILL(zTransaction)(&z3zE2442);
                              KILL(zTransaction)(&z3zE2443);
                              KILL(zTransaction)(&z3zE2444);
                              KILL(zTransaction)(&z3zE2445);
                              KILL(zTransaction)(&z3zE2446);
                              KILL(zTransaction)(&z3zE2447);
                              KILL(zTransaction)(&z3zE2448);
                              KILL(zTransaction)(&z3zE2449);
                              KILL(zTransaction)(&z3zE2450);
                              goto end_block_exception_2905;
                            }
                          }
                          struct zTransaction z3zE2451;
                          CREATE(zTransaction)(&z3zE2451);
                          {
                            struct zRlpFieldRef zs_f;
                            zs_f = z2zE2563.ztup0;
                            struct zByteSliceFields z1zE37;
                            z1zE37 = z2zE2563.ztup1;
                            unit z3zE2452;
                            {
                              z3zE2452 = zrlp_cursor_expect_end(z1zE37);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE2439);
                                KILL(zTransaction)(&z3zE2440);
                                KILL(zTransaction)(&z3zE2441);
                                KILL(zTransaction)(&z3zE2442);
                                KILL(zTransaction)(&z3zE2443);
                                KILL(zTransaction)(&z3zE2444);
                                KILL(zTransaction)(&z3zE2445);
                                KILL(zTransaction)(&z3zE2446);
                                KILL(zTransaction)(&z3zE2447);
                                KILL(zTransaction)(&z3zE2448);
                                KILL(zTransaction)(&z3zE2449);
                                KILL(zTransaction)(&z3zE2450);
                                KILL(zTransaction)(&z3zE2451);
                                goto end_block_exception_2905;
                              }
                            }
                            sail_u256 zv;
                            {
                              zv = zrlp_ref_word(zv_f);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE2439);
                                KILL(zTransaction)(&z3zE2440);
                                KILL(zTransaction)(&z3zE2441);
                                KILL(zTransaction)(&z3zE2442);
                                KILL(zTransaction)(&z3zE2443);
                                KILL(zTransaction)(&z3zE2444);
                                KILL(zTransaction)(&z3zE2445);
                                KILL(zTransaction)(&z3zE2446);
                                KILL(zTransaction)(&z3zE2447);
                                KILL(zTransaction)(&z3zE2448);
                                KILL(zTransaction)(&z3zE2449);
                                KILL(zTransaction)(&z3zE2450);
                                KILL(zTransaction)(&z3zE2451);
                                goto end_block_exception_2905;
                              }
                            }
                            struct ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9 z2zE2564;
                            CREATE(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2564);
                            {
                              zdecode_access_list(&z2zE2564, zal_f);
                              if (have_exception) {
                                KILL(zTransaction)(&z3zE2439);
                                KILL(zTransaction)(&z3zE2440);
                                KILL(zTransaction)(&z3zE2441);
                                KILL(zTransaction)(&z3zE2442);
                                KILL(zTransaction)(&z3zE2443);
                                KILL(zTransaction)(&z3zE2444);
                                KILL(zTransaction)(&z3zE2445);
                                KILL(zTransaction)(&z3zE2446);
                                KILL(zTransaction)(&z3zE2447);
                                KILL(zTransaction)(&z3zE2448);
                                KILL(zTransaction)(&z3zE2449);
                                KILL(zTransaction)(&z3zE2450);
                                KILL(zTransaction)(&z3zE2451);
                                KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2564);
                                goto end_block_exception_2905;
                              }
                            }
                            struct zTransaction z3zE2453;
                            CREATE(zTransaction)(&z3zE2453);
                            {
                              zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 zal_addrs;
                              CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                              COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs, z2zE2564.ztup0);
                              zz5listz8z5structz0zzStorageKeyz9 zal_slots;
                              CREATE(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                              COPY(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots, z2zE2564.ztup1);
                              uint64_t zal_addr_count;
                              zal_addr_count = z2zE2564.ztup2;
                              uint64_t zal_slot_count;
                              zal_slot_count = z2zE2564.ztup3;
                              struct ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9 z2zE2565;
                              CREATE(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE2565);
                              {
                                zdecode_auth_list(&z2zE2565, zauth_f);
                                if (have_exception) {
                                  KILL(zTransaction)(&z3zE2439);
                                  KILL(zTransaction)(&z3zE2440);
                                  KILL(zTransaction)(&z3zE2441);
                                  KILL(zTransaction)(&z3zE2442);
                                  KILL(zTransaction)(&z3zE2443);
                                  KILL(zTransaction)(&z3zE2444);
                                  KILL(zTransaction)(&z3zE2445);
                                  KILL(zTransaction)(&z3zE2446);
                                  KILL(zTransaction)(&z3zE2447);
                                  KILL(zTransaction)(&z3zE2448);
                                  KILL(zTransaction)(&z3zE2449);
                                  KILL(zTransaction)(&z3zE2450);
                                  KILL(zTransaction)(&z3zE2451);
                                  KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2564);
                                  KILL(zTransaction)(&z3zE2453);
                                  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                  KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                  KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE2565);
                                  goto end_block_exception_2905;
                                }
                              }
                              struct zTransaction z3zE2454;
                              CREATE(zTransaction)(&z3zE2454);
                              {
                                zz5listz8z5structz0zzAuthorizzzzationz9 zauthorizzations;
                                CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations, z2zE2565.ztup0);
                                uint64_t zauthorizzation_count;
                                zauthorizzation_count = z2zE2565.ztup1;
                                uint64_t z2zE2567;
                                {
                                  uint64_t z2zE2566;
                                  {
                                    z2zE2566 = zrlp_ref_uint64(znonce_f);
                                    if (have_exception) {
                                      KILL(zTransaction)(&z3zE2439);
                                      KILL(zTransaction)(&z3zE2440);
                                      KILL(zTransaction)(&z3zE2441);
                                      KILL(zTransaction)(&z3zE2442);
                                      KILL(zTransaction)(&z3zE2443);
                                      KILL(zTransaction)(&z3zE2444);
                                      KILL(zTransaction)(&z3zE2445);
                                      KILL(zTransaction)(&z3zE2446);
                                      KILL(zTransaction)(&z3zE2447);
                                      KILL(zTransaction)(&z3zE2448);
                                      KILL(zTransaction)(&z3zE2449);
                                      KILL(zTransaction)(&z3zE2450);
                                      KILL(zTransaction)(&z3zE2451);
                                      KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2564);
                                      KILL(zTransaction)(&z3zE2453);
                                      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                      KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                      KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE2565);
                                      KILL(zTransaction)(&z3zE2454);
                                      KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                      goto end_block_exception_2905;
                                    }
                                  }
                                  z2zE2567 = zword_of_account_nonce(z2zE2566);
                                }
                                uint64_t z2zE2568;
                                {
                                  z2zE2568 = zrlp_ref_uint64(zchain_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE2439);
                                    KILL(zTransaction)(&z3zE2440);
                                    KILL(zTransaction)(&z3zE2441);
                                    KILL(zTransaction)(&z3zE2442);
                                    KILL(zTransaction)(&z3zE2443);
                                    KILL(zTransaction)(&z3zE2444);
                                    KILL(zTransaction)(&z3zE2445);
                                    KILL(zTransaction)(&z3zE2446);
                                    KILL(zTransaction)(&z3zE2447);
                                    KILL(zTransaction)(&z3zE2448);
                                    KILL(zTransaction)(&z3zE2449);
                                    KILL(zTransaction)(&z3zE2450);
                                    KILL(zTransaction)(&z3zE2451);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2564);
                                    KILL(zTransaction)(&z3zE2453);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE2565);
                                    KILL(zTransaction)(&z3zE2454);
                                    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                    goto end_block_exception_2905;
                                  }
                                }
                                uint64_t z2zE2569;
                                {
                                  z2zE2569 = zrlp_ref_gas(zgas_f, zfork);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE2439);
                                    KILL(zTransaction)(&z3zE2440);
                                    KILL(zTransaction)(&z3zE2441);
                                    KILL(zTransaction)(&z3zE2442);
                                    KILL(zTransaction)(&z3zE2443);
                                    KILL(zTransaction)(&z3zE2444);
                                    KILL(zTransaction)(&z3zE2445);
                                    KILL(zTransaction)(&z3zE2446);
                                    KILL(zTransaction)(&z3zE2447);
                                    KILL(zTransaction)(&z3zE2448);
                                    KILL(zTransaction)(&z3zE2449);
                                    KILL(zTransaction)(&z3zE2450);
                                    KILL(zTransaction)(&z3zE2451);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2564);
                                    KILL(zTransaction)(&z3zE2453);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE2565);
                                    KILL(zTransaction)(&z3zE2454);
                                    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                    goto end_block_exception_2905;
                                  }
                                }
                                bool z2zE2571;
                                {
                                  uint64_t z2zE2570;
                                  z2zE2570 = zto_f.zcontent_len;
                                  z2zE2571 = (z2zE2570 == UINT64_C(0));
                                }
                                sail_fixed_bytes_20 z2zE2573;
                                {
                                  sail_u256 z2zE2572;
                                  {
                                    z2zE2572 = zrlp_ref_word(zto_f);
                                    if (have_exception) {
                                      KILL(zTransaction)(&z3zE2439);
                                      KILL(zTransaction)(&z3zE2440);
                                      KILL(zTransaction)(&z3zE2441);
                                      KILL(zTransaction)(&z3zE2442);
                                      KILL(zTransaction)(&z3zE2443);
                                      KILL(zTransaction)(&z3zE2444);
                                      KILL(zTransaction)(&z3zE2445);
                                      KILL(zTransaction)(&z3zE2446);
                                      KILL(zTransaction)(&z3zE2447);
                                      KILL(zTransaction)(&z3zE2448);
                                      KILL(zTransaction)(&z3zE2449);
                                      KILL(zTransaction)(&z3zE2450);
                                      KILL(zTransaction)(&z3zE2451);
                                      KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2564);
                                      KILL(zTransaction)(&z3zE2453);
                                      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                      KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                      KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE2565);
                                      KILL(zTransaction)(&z3zE2454);
                                      KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                      goto end_block_exception_2905;
                                    }
                                  }
                                  z2zE2573 = evmsail_word_to_address(z2zE2572);
                                }
                                sail_u256 z2zE2574;
                                {
                                  z2zE2574 = zrlp_ref_uint_word(zvalue_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE2439);
                                    KILL(zTransaction)(&z3zE2440);
                                    KILL(zTransaction)(&z3zE2441);
                                    KILL(zTransaction)(&z3zE2442);
                                    KILL(zTransaction)(&z3zE2443);
                                    KILL(zTransaction)(&z3zE2444);
                                    KILL(zTransaction)(&z3zE2445);
                                    KILL(zTransaction)(&z3zE2446);
                                    KILL(zTransaction)(&z3zE2447);
                                    KILL(zTransaction)(&z3zE2448);
                                    KILL(zTransaction)(&z3zE2449);
                                    KILL(zTransaction)(&z3zE2450);
                                    KILL(zTransaction)(&z3zE2451);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2564);
                                    KILL(zTransaction)(&z3zE2453);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE2565);
                                    KILL(zTransaction)(&z3zE2454);
                                    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                    goto end_block_exception_2905;
                                  }
                                }
                                struct zByteSliceFields z2zE2575;
                                {
                                  z2zE2575 = ztx_input_span(zdata_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE2439);
                                    KILL(zTransaction)(&z3zE2440);
                                    KILL(zTransaction)(&z3zE2441);
                                    KILL(zTransaction)(&z3zE2442);
                                    KILL(zTransaction)(&z3zE2443);
                                    KILL(zTransaction)(&z3zE2444);
                                    KILL(zTransaction)(&z3zE2445);
                                    KILL(zTransaction)(&z3zE2446);
                                    KILL(zTransaction)(&z3zE2447);
                                    KILL(zTransaction)(&z3zE2448);
                                    KILL(zTransaction)(&z3zE2449);
                                    KILL(zTransaction)(&z3zE2450);
                                    KILL(zTransaction)(&z3zE2451);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2564);
                                    KILL(zTransaction)(&z3zE2453);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE2565);
                                    KILL(zTransaction)(&z3zE2454);
                                    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                    goto end_block_exception_2905;
                                  }
                                }
                                sail_u256 z2zE2576;
                                {
                                  z2zE2576 = zrlp_ref_uint_word(zmf_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE2439);
                                    KILL(zTransaction)(&z3zE2440);
                                    KILL(zTransaction)(&z3zE2441);
                                    KILL(zTransaction)(&z3zE2442);
                                    KILL(zTransaction)(&z3zE2443);
                                    KILL(zTransaction)(&z3zE2444);
                                    KILL(zTransaction)(&z3zE2445);
                                    KILL(zTransaction)(&z3zE2446);
                                    KILL(zTransaction)(&z3zE2447);
                                    KILL(zTransaction)(&z3zE2448);
                                    KILL(zTransaction)(&z3zE2449);
                                    KILL(zTransaction)(&z3zE2450);
                                    KILL(zTransaction)(&z3zE2451);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2564);
                                    KILL(zTransaction)(&z3zE2453);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE2565);
                                    KILL(zTransaction)(&z3zE2454);
                                    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                    goto end_block_exception_2905;
                                  }
                                }
                                sail_u256 z2zE2577;
                                {
                                  z2zE2577 = zrlp_ref_uint_word(zmp_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE2439);
                                    KILL(zTransaction)(&z3zE2440);
                                    KILL(zTransaction)(&z3zE2441);
                                    KILL(zTransaction)(&z3zE2442);
                                    KILL(zTransaction)(&z3zE2443);
                                    KILL(zTransaction)(&z3zE2444);
                                    KILL(zTransaction)(&z3zE2445);
                                    KILL(zTransaction)(&z3zE2446);
                                    KILL(zTransaction)(&z3zE2447);
                                    KILL(zTransaction)(&z3zE2448);
                                    KILL(zTransaction)(&z3zE2449);
                                    KILL(zTransaction)(&z3zE2450);
                                    KILL(zTransaction)(&z3zE2451);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2564);
                                    KILL(zTransaction)(&z3zE2453);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE2565);
                                    KILL(zTransaction)(&z3zE2454);
                                    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                    goto end_block_exception_2905;
                                  }
                                }
                                sail_fixed_bytes_32 z2zE2579;
                                {
                                  struct zByteSliceFields z2zE2578;
                                  {
                                    z2zE2578 = ztx_sig_span(zchain_f, zv_f);
                                    if (have_exception) {
                                      KILL(zTransaction)(&z3zE2439);
                                      KILL(zTransaction)(&z3zE2440);
                                      KILL(zTransaction)(&z3zE2441);
                                      KILL(zTransaction)(&z3zE2442);
                                      KILL(zTransaction)(&z3zE2443);
                                      KILL(zTransaction)(&z3zE2444);
                                      KILL(zTransaction)(&z3zE2445);
                                      KILL(zTransaction)(&z3zE2446);
                                      KILL(zTransaction)(&z3zE2447);
                                      KILL(zTransaction)(&z3zE2448);
                                      KILL(zTransaction)(&z3zE2449);
                                      KILL(zTransaction)(&z3zE2450);
                                      KILL(zTransaction)(&z3zE2451);
                                      KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2564);
                                      KILL(zTransaction)(&z3zE2453);
                                      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                      KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                      KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE2565);
                                      KILL(zTransaction)(&z3zE2454);
                                      KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                      goto end_block_exception_2905;
                                    }
                                  }
                                  {
                                    z2zE2579 = ztx_signing_hash(zSetCodeTx, z2zE2578, zv);
                                    if (have_exception) {
                                      KILL(zTransaction)(&z3zE2439);
                                      KILL(zTransaction)(&z3zE2440);
                                      KILL(zTransaction)(&z3zE2441);
                                      KILL(zTransaction)(&z3zE2442);
                                      KILL(zTransaction)(&z3zE2443);
                                      KILL(zTransaction)(&z3zE2444);
                                      KILL(zTransaction)(&z3zE2445);
                                      KILL(zTransaction)(&z3zE2446);
                                      KILL(zTransaction)(&z3zE2447);
                                      KILL(zTransaction)(&z3zE2448);
                                      KILL(zTransaction)(&z3zE2449);
                                      KILL(zTransaction)(&z3zE2450);
                                      KILL(zTransaction)(&z3zE2451);
                                      KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2564);
                                      KILL(zTransaction)(&z3zE2453);
                                      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                      KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                      KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE2565);
                                      KILL(zTransaction)(&z3zE2454);
                                      KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                      goto end_block_exception_2905;
                                    }
                                  }
                                }
                                sail_u256 z2zE2580;
                                {
                                  z2zE2580 = zrlp_ref_uint_word(zr_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE2439);
                                    KILL(zTransaction)(&z3zE2440);
                                    KILL(zTransaction)(&z3zE2441);
                                    KILL(zTransaction)(&z3zE2442);
                                    KILL(zTransaction)(&z3zE2443);
                                    KILL(zTransaction)(&z3zE2444);
                                    KILL(zTransaction)(&z3zE2445);
                                    KILL(zTransaction)(&z3zE2446);
                                    KILL(zTransaction)(&z3zE2447);
                                    KILL(zTransaction)(&z3zE2448);
                                    KILL(zTransaction)(&z3zE2449);
                                    KILL(zTransaction)(&z3zE2450);
                                    KILL(zTransaction)(&z3zE2451);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2564);
                                    KILL(zTransaction)(&z3zE2453);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE2565);
                                    KILL(zTransaction)(&z3zE2454);
                                    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                    goto end_block_exception_2905;
                                  }
                                }
                                sail_u256 z2zE2581;
                                {
                                  z2zE2581 = zrlp_ref_uint_word(zs_f);
                                  if (have_exception) {
                                    KILL(zTransaction)(&z3zE2439);
                                    KILL(zTransaction)(&z3zE2440);
                                    KILL(zTransaction)(&z3zE2441);
                                    KILL(zTransaction)(&z3zE2442);
                                    KILL(zTransaction)(&z3zE2443);
                                    KILL(zTransaction)(&z3zE2444);
                                    KILL(zTransaction)(&z3zE2445);
                                    KILL(zTransaction)(&z3zE2446);
                                    KILL(zTransaction)(&z3zE2447);
                                    KILL(zTransaction)(&z3zE2448);
                                    KILL(zTransaction)(&z3zE2449);
                                    KILL(zTransaction)(&z3zE2450);
                                    KILL(zTransaction)(&z3zE2451);
                                    KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2564);
                                    KILL(zTransaction)(&z3zE2453);
                                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                                    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                                    KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE2565);
                                    KILL(zTransaction)(&z3zE2454);
                                    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                    goto end_block_exception_2905;
                                  }
                                }
                                z3zE2454.zaccess_list_address_count = zal_addr_count;
                                COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE2454)->zaccess_list_addresses), zal_addrs);
                                z3zE2454.zaccess_list_slot_count = zal_slot_count;
                                COPY(zz5listz8z5structz0zzStorageKeyz9)(&((&z3zE2454)->zaccess_list_slots), zal_slots);
                                z3zE2454.zauthorizzation_count = zauthorizzation_count;
                                COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&((&z3zE2454)->zauthorizzations), zauthorizzations);
                                z3zE2454.zblob_hashes = zEMPTY_BLOB_HASHES;
                                z3zE2454.zchain_id = z2zE2568;
                                z3zE2454.zgas_limit = z2zE2569;
                                z3zE2454.zinput_src = z2zE2575;
                                z3zE2454.zis_create = z2zE2571;
                                z3zE2454.zmax_blob_fee = zZERO_WORD;
                                z3zE2454.zmax_fee = z2zE2576;
                                z3zE2454.zmax_priority_fee = z2zE2577;
                                z3zE2454.znonce = u256_of_fbits(z2zE2567);
                                z3zE2454.zpubkey = zpubkey;
                                z3zE2454.zraw = ztx;
                                z3zE2454.zrecipient = z2zE2573;
                                z3zE2454.zsender = zsender;
                                z3zE2454.zsig_r = z2zE2580;
                                z3zE2454.zsig_s = z2zE2581;
                                z3zE2454.zsig_v = zv;
                                z3zE2454.zsigning_hash = z2zE2579;
                                z3zE2454.ztx_type = zSetCodeTx;
                                z3zE2454.zvalue = z2zE2574;
                                KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zauthorizzations);
                                goto finish_match_2902;
                              }
                            case_2903: ;
                              sail_match_failure("decode_set_code_tx");
                            finish_match_2902: ;
                              COPY(zTransaction)(&z3zE2453, z3zE2454);
                              KILL(zTransaction)(&z3zE2454);
                              KILL(ztuple_z8z5listz8z5structz0zzAuthorizzzzationz9zCz0z5u64z9)(&z2zE2565);
                              KILL(zz5listz8z5structz0zzStorageKeyz9)(&zal_slots);
                              KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zal_addrs);
                              goto finish_match_2900;
                            }
                          case_2901: ;
                            sail_match_failure("decode_set_code_tx");
                          finish_match_2900: ;
                            COPY(zTransaction)(&z3zE2451, z3zE2453);
                            KILL(zTransaction)(&z3zE2453);
                            KILL(ztuple_z8z5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9zCz0z5listz8z5structz0zzStorageKeyz9zCz0z5u64zCz0z5u64z9)(&z2zE2564);
                            goto finish_match_2898;
                          }
                        case_2899: ;
                          sail_match_failure("decode_set_code_tx");
                        finish_match_2898: ;
                          COPY(zTransaction)(&z3zE2450, z3zE2451);
                          KILL(zTransaction)(&z3zE2451);
                          goto finish_match_2896;
                        }
                      case_2897: ;
                        sail_match_failure("decode_set_code_tx");
                      finish_match_2896: ;
                        COPY(zTransaction)(&z3zE2449, z3zE2450);
                        KILL(zTransaction)(&z3zE2450);
                        goto finish_match_2894;
                      }
                    case_2895: ;
                      sail_match_failure("decode_set_code_tx");
                    finish_match_2894: ;
                      COPY(zTransaction)(&z3zE2448, z3zE2449);
                      KILL(zTransaction)(&z3zE2449);
                      goto finish_match_2892;
                    }
                  case_2893: ;
                    sail_match_failure("decode_set_code_tx");
                  finish_match_2892: ;
                    COPY(zTransaction)(&z3zE2447, z3zE2448);
                    KILL(zTransaction)(&z3zE2448);
                    goto finish_match_2890;
                  }
                case_2891: ;
                  sail_match_failure("decode_set_code_tx");
                finish_match_2890: ;
                  COPY(zTransaction)(&z3zE2446, z3zE2447);
                  KILL(zTransaction)(&z3zE2447);
                  goto finish_match_2888;
                }
              case_2889: ;
                sail_match_failure("decode_set_code_tx");
              finish_match_2888: ;
                COPY(zTransaction)(&z3zE2445, z3zE2446);
                KILL(zTransaction)(&z3zE2446);
                goto finish_match_2886;
              }
            case_2887: ;
              sail_match_failure("decode_set_code_tx");
            finish_match_2886: ;
              COPY(zTransaction)(&z3zE2444, z3zE2445);
              KILL(zTransaction)(&z3zE2445);
              goto finish_match_2884;
            }
          case_2885: ;
            sail_match_failure("decode_set_code_tx");
          finish_match_2884: ;
            COPY(zTransaction)(&z3zE2443, z3zE2444);
            KILL(zTransaction)(&z3zE2444);
            goto finish_match_2882;
          }
        case_2883: ;
          sail_match_failure("decode_set_code_tx");
        finish_match_2882: ;
          COPY(zTransaction)(&z3zE2442, z3zE2443);
          KILL(zTransaction)(&z3zE2443);
          goto finish_match_2880;
        }
      case_2881: ;
        sail_match_failure("decode_set_code_tx");
      finish_match_2880: ;
        COPY(zTransaction)(&z3zE2441, z3zE2442);
        KILL(zTransaction)(&z3zE2442);
        goto finish_match_2878;
      }
    case_2879: ;
      sail_match_failure("decode_set_code_tx");
    finish_match_2878: ;
      COPY(zTransaction)(&z3zE2440, z3zE2441);
      KILL(zTransaction)(&z3zE2441);
      goto finish_match_2876;
    }
  case_2877: ;
    sail_match_failure("decode_set_code_tx");
  finish_match_2876: ;
    COPY(zTransaction)(&z3zE2439, z3zE2440);
    KILL(zTransaction)(&z3zE2440);
    goto finish_match_2874;
  }
case_2875: ;
  sail_match_failure("decode_set_code_tx");
finish_match_2874: ;
  COPY(zTransaction)((*(&z8zE200)), z3zE2439);
  KILL(zTransaction)(&z3zE2439);
end_function_2904: ;
  goto end_function_3643;
end_block_exception_2905: ;
  goto end_function_3643;
end_function_3643: ;
}

void zrlp_decode_tx(struct zTransaction *z8zE201, struct zByteSliceFields ztx, struct zByteSliceFields zpubkey, enum zFork zfork)
{
  sail_fixed_bytes_20 zsender;
  {
    sail_u256 z2zE2550;
    {
      sail_fixed_bytes_32 z2zE2549;
      {
        struct zByteSliceFields z2zE2548;
        z2zE2548 = zsub_slice(zpubkey, UINT64_C(1), zPUBLIC_KEY_BODY_LENGTH);
        z2zE2549 = zkeccak256_slice(z2zE2548);
      }
      z2zE2550 = evmsail_hash_to_word(z2zE2549);
    }
    zsender = evmsail_word_to_address(z2zE2550);
  }
  uint64_t ztx_length;
  ztx_length = ztx.zlen;
  uint64_t zb0;
  {
    bool z2zE2546;
    z2zE2546 = (ztx_length == UINT64_C(0));
    if (z2zE2546) {
      struct zexception z2zE2547;
      CREATE(zexception)(&z2zE2547);
      zInvalidBlock(&z2zE2547, zRlpDecode);
      COPY(zexception)(current_exception, z2zE2547);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/lib/rlp/tx.sail:624.12-624.41");
      KILL(zexception)(&z2zE2547);
      goto end_block_exception_2873;
      /* unreachable after throw */
      KILL(zexception)(&z2zE2547);
    } else {  zb0 = zslice_byte(ztx, UINT64_C(0));  }
  }
  uint64_t zttype;
  {
    bool z2zE2545;
    {
      uint64_t z2zE2544;
      z2zE2544 = (safe_rshift(UINT64_MAX, 64 - 2) & (zb0 >> UINT64_C(6)));
      z2zE2545 = (z2zE2544 == UINT64_C(0b11));
    }
    if (z2zE2545) {  zttype = UINT64_C(0x00);  } else {  zttype = zb0;  }
  }
  bool ztyped;
  ztyped = (zttype != UINT64_C(0x00));
  struct zByteSliceFields zpayload;
  if (ztyped) {
    bool z2zE2541;
    z2zE2541 = (!(ztx_length < UINT64_C(1)));
    if (z2zE2541) {
      uint64_t z2zE2542;
      {    z2zE2542 = (ztx_length - UINT64_C(1));
      }
      zpayload = zsub_slice(ztx, UINT64_C(1), z2zE2542);
    } else {
      struct zexception z2zE2543;
      CREATE(zexception)(&z2zE2543);
      zInvalidBlock(&z2zE2543, zRlpDecode);
      COPY(zexception)(current_exception, z2zE2543);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/lib/rlp/tx.sail:634.16-634.45");
      KILL(zexception)(&z2zE2543);
      goto end_block_exception_2873;
      /* unreachable after throw */
      KILL(zexception)(&z2zE2543);
    }
  } else {  zpayload = ztx;  }
  struct zByteSliceFields zfields;
  {
    zfields = zrlp_node_cursor(zpayload);
    if (have_exception) {  goto end_block_exception_2873;  }
  }
  struct zTransaction z3zE2428;
  CREATE(zTransaction)(&z3zE2428);
  {
    uint64_t zp0z3;
    zp0z3 = zttype;
    bool z3zE2434;
    z3zE2434 = (zp0z3 == UINT64_C(0x00));
    if (!(z3zE2434)) {  goto case_2871;  }
    {
      zdecode_legacy_tx(&z3zE2428, ztx, zpubkey, zfork, zsender, zfields);
      if (have_exception) {
        KILL(zTransaction)(&z3zE2428);
        goto end_block_exception_2873;
      }
    }
    goto finish_match_2865;
  }
case_2871: ;
  {
    uint64_t z3zE2435;
    z3zE2435 = zttype;
    bool z3zE2433;
    z3zE2433 = (z3zE2435 == UINT64_C(0x01));
    if (!(z3zE2433)) {  goto case_2870;  }
    {
      zdecode_access_list_tx(&z3zE2428, ztx, zpubkey, zfork, zsender, zfields);
      if (have_exception) {
        KILL(zTransaction)(&z3zE2428);
        goto end_block_exception_2873;
      }
    }
    goto finish_match_2865;
  }
case_2870: ;
  {
    uint64_t z3zE2436;
    z3zE2436 = zttype;
    bool z3zE2432;
    z3zE2432 = (z3zE2436 == UINT64_C(0x02));
    if (!(z3zE2432)) {  goto case_2869;  }
    {
      zdecode_fee_market_tx(&z3zE2428, ztx, zpubkey, zfork, zsender, zfields);
      if (have_exception) {
        KILL(zTransaction)(&z3zE2428);
        goto end_block_exception_2873;
      }
    }
    goto finish_match_2865;
  }
case_2869: ;
  {
    uint64_t z3zE2437;
    z3zE2437 = zttype;
    bool z3zE2431;
    z3zE2431 = (z3zE2437 == UINT64_C(0x03));
    if (!(z3zE2431)) {  goto case_2868;  }
    {
      zdecode_blob_tx(&z3zE2428, ztx, zpubkey, zfork, zsender, zfields);
      if (have_exception) {
        KILL(zTransaction)(&z3zE2428);
        goto end_block_exception_2873;
      }
    }
    goto finish_match_2865;
  }
case_2868: ;
  {
    uint64_t z3zE2438;
    z3zE2438 = zttype;
    bool z3zE2430;
    z3zE2430 = (z3zE2438 == UINT64_C(0x04));
    if (!(z3zE2430)) {  goto case_2867;  }
    {
      zdecode_set_code_tx(&z3zE2428, ztx, zpubkey, zfork, zsender, zfields);
      if (have_exception) {
        KILL(zTransaction)(&z3zE2428);
        goto end_block_exception_2873;
      }
    }
    goto finish_match_2865;
  }
case_2867: ;
  {
    struct zexception z2zE2540;
    CREATE(zexception)(&z2zE2540);
    zInvalidBlock(&z2zE2540, zRlpDecode);
    COPY(zexception)(current_exception, z2zE2540);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/rlp/tx.sail:647.13-647.42");
    KILL(zTransaction)(&z3zE2428);
    KILL(zexception)(&z2zE2540);
    goto end_block_exception_2873;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2540);
    goto finish_match_2865;
  }
case_2866: ;
finish_match_2865: ;
  COPY(zTransaction)((*(&z8zE201)), z3zE2428);
  KILL(zTransaction)(&z3zE2428);
end_function_2872: ;
  goto end_function_3642;
end_block_exception_2873: ;
  goto end_function_3642;
end_function_3642: ;
}

