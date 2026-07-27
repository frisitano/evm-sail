/* Generated from sail/executor/block_access_list.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
uint64_t zbal_count_item(uint64_t zcount, uint64_t zmaximum)
{
  uint64_t z8zE662;
  bool z2zE350;
  z2zE350 = (zcount < zmaximum);
  if (z2zE350) {
    {    z8zE662 = (zcount + UINT64_C(1));
    }
  } else {
    struct zexception z2zE351;
    CREATE(zexception)(&z2zE351);
    zInvalidBlock(&z2zE351, zBlockAccessListTooLarge);
    COPY(zexception)(current_exception, z2zE351);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/block_access_list.sail:17.8-17.51");
    KILL(zexception)(&z2zE351);
    goto end_block_exception_1274;
    /* unreachable after throw */
    KILL(zexception)(&z2zE351);
  }
end_function_1273: ;
  return z8zE662;
end_block_exception_1274: ;

  return UINT64_C(0xdeadc0de);
}

struct zByteSliceFields zbal_ref_cursor(struct zRlpFieldRef zf)
{
  struct zByteSliceFields z8zE663;
  bool z2zE348;
  {
    bool z2zE347;
    z2zE347 = zf.zis_list;
    bool z3zE674;
    if (z2zE347) {  z3zE674 = zrlp_ref_framing_canonical(zf);  } else {  z3zE674 = false;  }
    z2zE348 = z3zE674;
  }
  if (z2zE348) {
    {
      z8zE663 = zrlp_ref_cursor(zf);
      if (have_exception) {  goto end_block_exception_1272;  }
    }
  } else {
    struct zexception z2zE349;
    CREATE(zexception)(&z2zE349);
    zInvalidBlock(&z2zE349, zInvalidBlockAccessList);
    COPY(zexception)(current_exception, z2zE349);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/block_access_list.sail:31.8-31.50");
    KILL(zexception)(&z2zE349);
    goto end_block_exception_1272;
    /* unreachable after throw */
    KILL(zexception)(&z2zE349);
  }
end_function_1271: ;
  return z8zE663;
end_block_exception_1272: ;
  struct zByteSliceFields z8zE1004 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE1004;
}

struct zByteSliceFields zbal_ref_bytes(struct zRlpFieldRef zf)
{
  struct zByteSliceFields z8zE664;
  bool z2zE345;
  z2zE345 = zrlp_ref_bytes_canonical(zf);
  if (z2zE345) {  z8zE664 = zrlp_ref_content(zf);  } else {
    struct zexception z2zE346;
    CREATE(zexception)(&z2zE346);
    zInvalidBlock(&z2zE346, zInvalidBlockAccessList);
    COPY(zexception)(current_exception, z2zE346);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/block_access_list.sail:43.8-43.50");
    KILL(zexception)(&z2zE346);
    goto end_block_exception_1270;
    /* unreachable after throw */
    KILL(zexception)(&z2zE346);
  }
end_function_1269: ;
  return z8zE664;
end_block_exception_1270: ;
  struct zByteSliceFields z8zE1005 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE1005;
}

sail_u256 zbal_ref_word(struct zRlpFieldRef zf)
{
  sail_u256 z8zE665;
  bool z2zE343;
  {
    bool z2zE342;
    z2zE342 = zrlp_ref_uint_canonical(zf);
    bool z3zE673;
    if (z2zE342) {
      uint64_t z2zE341;
      z2zE341 = zf.zcontent_len;
      z3zE673 = (!(UINT64_C(32) < z2zE341));
    } else {  z3zE673 = false;  }
    z2zE343 = z3zE673;
  }
  if (z2zE343) {
    {
      z8zE665 = zrlp_ref_word(zf);
      if (have_exception) {  goto end_block_exception_1268;  }
    }
  } else {
    struct zexception z2zE344;
    CREATE(zexception)(&z2zE344);
    zInvalidBlock(&z2zE344, zInvalidBlockAccessList);
    COPY(zexception)(current_exception, z2zE344);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/block_access_list.sail:55.8-55.50");
    KILL(zexception)(&z2zE344);
    goto end_block_exception_1268;
    /* unreachable after throw */
    KILL(zexception)(&z2zE344);
  }
end_function_1267: ;
  return z8zE665;
end_block_exception_1268: ;

  return u256_zero();
}

uint64_t zbal_ref_uint64(struct zRlpFieldRef zf)
{
  uint64_t z8zE666;
  bool z2zE339;
  {
    bool z2zE338;
    z2zE338 = zrlp_ref_uint_canonical(zf);
    bool z3zE672;
    if (z2zE338) {
      uint64_t z2zE337;
      z2zE337 = zf.zcontent_len;
      z3zE672 = (!(UINT64_C(8) < z2zE337));
    } else {  z3zE672 = false;  }
    z2zE339 = z3zE672;
  }
  if (z2zE339) {
    {
      z8zE666 = zrlp_ref_uint64(zf);
      if (have_exception) {  goto end_block_exception_1266;  }
    }
  } else {
    struct zexception z2zE340;
    CREATE(zexception)(&z2zE340);
    zInvalidBlock(&z2zE340, zInvalidBlockAccessList);
    COPY(zexception)(current_exception, z2zE340);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/block_access_list.sail:67.8-67.50");
    KILL(zexception)(&z2zE340);
    goto end_block_exception_1266;
    /* unreachable after throw */
    KILL(zexception)(&z2zE340);
  }
end_function_1265: ;
  return z8zE666;
end_block_exception_1266: ;

  return UINT64_C(0xdeadc0de);
}

unit zbal_expect_end(struct zByteSliceFields zcursor)
{
  unit z8zE667;
  bool z2zE335;
  {
    uint64_t z2zE334;
    z2zE334 = zcursor.zlen;
    z2zE335 = (z2zE334 == UINT64_C(0));
  }
  if (z2zE335) {  z8zE667 = UNIT;  } else {
    struct zexception z2zE336;
    CREATE(zexception)(&z2zE336);
    zInvalidBlock(&z2zE336, zInvalidBlockAccessList);
    COPY(zexception)(current_exception, z2zE336);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/block_access_list.sail:78.8-78.50");
    KILL(zexception)(&z2zE336);
    goto end_block_exception_1264;
    /* unreachable after throw */
    KILL(zexception)(&z2zE336);
  }
end_function_1263: ;
  return z8zE667;
end_block_exception_1264: ;

  return UNIT;
}

unit zbal_compare_index_word(struct zRlpFieldRef zpair, uint64_t zindex, sail_u256 zvalue)
{
  unit z8zE668;
  struct zByteSliceFields zfields;
  {
    zfields = zbal_ref_cursor(zpair);
    if (have_exception) {  goto end_block_exception_1262;  }
  }
  struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE327;
  {
    z2zE327 = zrlp_cursor_pop(zfields);
    if (have_exception) {  goto end_block_exception_1262;  }
  }
  unit z3zE666;
  {
    struct zRlpFieldRef zindex_field;
    zindex_field = z2zE327.ztup0;
    struct zByteSliceFields z1zE18;
    z1zE18 = z2zE327.ztup1;
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE328;
    {
      z2zE328 = zrlp_cursor_pop(z1zE18);
      if (have_exception) {  goto end_block_exception_1262;  }
    }
    unit z3zE667;
    {
      struct zRlpFieldRef zvalue_field;
      zvalue_field = z2zE328.ztup0;
      struct zByteSliceFields z1zE19;
      z1zE19 = z2zE328.ztup1;
      unit z3zE668;
      {
        z3zE668 = zbal_expect_end(z1zE19);
        if (have_exception) {  goto end_block_exception_1262;  }
      }
      bool z2zE332;
      {
        bool z2zE331;
        {
          uint64_t z2zE329;
          {
            z2zE329 = zbal_ref_uint64(zindex_field);
            if (have_exception) {  goto end_block_exception_1262;  }
          }
          z2zE331 = (z2zE329 != zindex);
        }
        bool z3zE669;
        if (z2zE331) {  z3zE669 = true;  } else {
          sail_u256 z2zE330;
          {
            z2zE330 = zbal_ref_word(zvalue_field);
            if (have_exception) {  goto end_block_exception_1262;  }
          }
          z3zE669 = (!eq_u256(z2zE330, zvalue));
        }
        z2zE332 = z3zE669;
      }
      if (z2zE332) {
        struct zexception z2zE333;
        CREATE(zexception)(&z2zE333);
        zInvalidBlock(&z2zE333, zInvalidBlockAccessList);
        COPY(zexception)(current_exception, z2zE333);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/executor/block_access_list.sail:95.8-95.50");
        KILL(zexception)(&z2zE333);
        goto end_block_exception_1262;
        /* unreachable after throw */
        KILL(zexception)(&z2zE333);
      } else {  z3zE667 = UNIT;  }
      goto finish_match_1259;
    }
  case_1260: ;
    sail_match_failure("bal_compare_index_word");
  finish_match_1259: ;
    z3zE666 = z3zE667;
    goto finish_match_1257;
  }
case_1258: ;
  sail_match_failure("bal_compare_index_word");
finish_match_1257: ;
  z8zE668 = z3zE666;
end_function_1261: ;
  return z8zE668;
end_block_exception_1262: ;

  return UNIT;
}

unit zbal_compare_index_nonce(struct zRlpFieldRef zpair, uint64_t zindex, uint64_t zvalue)
{
  unit z8zE669;
  struct zByteSliceFields zfields;
  {
    zfields = zbal_ref_cursor(zpair);
    if (have_exception) {  goto end_block_exception_1256;  }
  }
  struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE320;
  {
    z2zE320 = zrlp_cursor_pop(zfields);
    if (have_exception) {  goto end_block_exception_1256;  }
  }
  unit z3zE660;
  {
    struct zRlpFieldRef zindex_field;
    zindex_field = z2zE320.ztup0;
    struct zByteSliceFields z1zE16;
    z1zE16 = z2zE320.ztup1;
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE321;
    {
      z2zE321 = zrlp_cursor_pop(z1zE16);
      if (have_exception) {  goto end_block_exception_1256;  }
    }
    unit z3zE661;
    {
      struct zRlpFieldRef zvalue_field;
      zvalue_field = z2zE321.ztup0;
      struct zByteSliceFields z1zE17;
      z1zE17 = z2zE321.ztup1;
      unit z3zE662;
      {
        z3zE662 = zbal_expect_end(z1zE17);
        if (have_exception) {  goto end_block_exception_1256;  }
      }
      bool z2zE325;
      {
        bool z2zE324;
        {
          uint64_t z2zE322;
          {
            z2zE322 = zbal_ref_uint64(zindex_field);
            if (have_exception) {  goto end_block_exception_1256;  }
          }
          z2zE324 = (z2zE322 != zindex);
        }
        bool z3zE663;
        if (z2zE324) {  z3zE663 = true;  } else {
          uint64_t z2zE323;
          {
            z2zE323 = zbal_ref_uint64(zvalue_field);
            if (have_exception) {  goto end_block_exception_1256;  }
          }
          z3zE663 = (z2zE323 != zvalue);
        }
        z2zE325 = z3zE663;
      }
      if (z2zE325) {
        struct zexception z2zE326;
        CREATE(zexception)(&z2zE326);
        zInvalidBlock(&z2zE326, zInvalidBlockAccessList);
        COPY(zexception)(current_exception, z2zE326);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/executor/block_access_list.sail:113.8-113.50");
        KILL(zexception)(&z2zE326);
        goto end_block_exception_1256;
        /* unreachable after throw */
        KILL(zexception)(&z2zE326);
      } else {  z3zE661 = UNIT;  }
      goto finish_match_1253;
    }
  case_1254: ;
    sail_match_failure("bal_compare_index_nonce");
  finish_match_1253: ;
    z3zE660 = z3zE661;
    goto finish_match_1251;
  }
case_1252: ;
  sail_match_failure("bal_compare_index_nonce");
finish_match_1251: ;
  z8zE669 = z3zE660;
end_function_1255: ;
  return z8zE669;
end_block_exception_1256: ;

  return UNIT;
}

unit zbal_compare_index_code(struct zRlpFieldRef zpair, uint64_t zindex, sail_fixed_bytes_32 zcode_hash)
{
  unit z8zE670;
  struct zByteSliceFields zfields;
  {
    zfields = zbal_ref_cursor(zpair);
    if (have_exception) {  goto end_block_exception_1250;  }
  }
  struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE311;
  {
    z2zE311 = zrlp_cursor_pop(zfields);
    if (have_exception) {  goto end_block_exception_1250;  }
  }
  unit z3zE654;
  {
    struct zRlpFieldRef zindex_field;
    zindex_field = z2zE311.ztup0;
    struct zByteSliceFields z1zE14;
    z1zE14 = z2zE311.ztup1;
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE312;
    {
      z2zE312 = zrlp_cursor_pop(z1zE14);
      if (have_exception) {  goto end_block_exception_1250;  }
    }
    unit z3zE655;
    {
      struct zRlpFieldRef zcode_field;
      zcode_field = z2zE312.ztup0;
      struct zByteSliceFields z1zE15;
      z1zE15 = z2zE312.ztup1;
      unit z3zE656;
      {
        z3zE656 = zbal_expect_end(z1zE15);
        if (have_exception) {  goto end_block_exception_1250;  }
      }
      struct zCode zcode;
      {
        zcode = zcode_db_resolve(zcode_hash);
        if (have_exception) {  goto end_block_exception_1250;  }
      }
      bool z2zE318;
      {
        bool z2zE317;
        {
          uint64_t z2zE313;
          {
            z2zE313 = zbal_ref_uint64(zindex_field);
            if (have_exception) {  goto end_block_exception_1250;  }
          }
          z2zE317 = (z2zE313 != zindex);
        }
        bool z3zE657;
        if (z2zE317) {  z3zE657 = true;  } else {
          bool z2zE316;
          {
            struct zByteSliceFields z2zE314;
            {
              z2zE314 = zbal_ref_bytes(zcode_field);
              if (have_exception) {  goto end_block_exception_1250;  }
            }
            struct zByteSliceFields z2zE315;
            z2zE315 = zcode.zbytes;
            z2zE316 = host_byte_slices_equal(z2zE314, z2zE315);
          }
          z3zE657 = not(z2zE316);
        }
        z2zE318 = z3zE657;
      }
      if (z2zE318) {
        struct zexception z2zE319;
        CREATE(zexception)(&z2zE319);
        zInvalidBlock(&z2zE319, zInvalidBlockAccessList);
        COPY(zexception)(current_exception, z2zE319);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/executor/block_access_list.sail:133.8-133.50");
        KILL(zexception)(&z2zE319);
        goto end_block_exception_1250;
        /* unreachable after throw */
        KILL(zexception)(&z2zE319);
      } else {  z3zE655 = UNIT;  }
      goto finish_match_1247;
    }
  case_1248: ;
    sail_match_failure("bal_compare_index_code");
  finish_match_1247: ;
    z3zE654 = z3zE655;
    goto finish_match_1245;
  }
case_1246: ;
  sail_match_failure("bal_compare_index_code");
finish_match_1245: ;
  z8zE670 = z3zE654;
end_function_1249: ;
  return z8zE670;
end_block_exception_1250: ;

  return UNIT;
}

unit zbal_validate_storage_change_values(struct zByteSliceFields zcursor, sail_u256 zslot)
{
  unit z8zE671;
  bool z2zE302;
  {
    uint64_t z2zE301;
    z2zE301 = zcursor.zlen;
    z2zE302 = (z2zE301 == UINT64_C(0));
  }
  if (z2zE302) {  z8zE671 = UNIT;  } else {
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE303;
    {
      z2zE303 = zrlp_cursor_pop(zcursor);
      if (have_exception) {  goto end_block_exception_1244;  }
    }
    unit z3zE647;
    {
      struct zRlpFieldRef zpair;
      zpair = z2zE303.ztup0;
      struct zByteSliceFields znext;
      znext = z2zE303.ztup1;
      struct zBalIterEntry z2zE304;
      CREATE(zBalIterEntry)(&z2zE304);
      bal_iter_next(&z2zE304, UNIT);
      unit z3zE648;
      {
        if (z2zE304.kind != Kind_zBalStorageChange) goto case_1242;
        struct zBalStorageChangeEntry zchange;
        zchange = z2zE304.variants.zBalStorageChange;
        bool z2zE306;
        {
          sail_u256 z2zE305;
          z2zE305 = zchange.zslot;
          z2zE306 = (!eq_u256(z2zE305, zslot));
        }
        unit z3zE650;
        if (z2zE306) {
          struct zexception z2zE307;
          CREATE(zexception)(&z2zE307);
          zInvalidBlock(&z2zE307, zInvalidBlockAccessList);
          COPY(zexception)(current_exception, z2zE307);
          have_exception = true;
          COPY(sail_string)(throw_location, "sail/executor/block_access_list.sail:153.20-153.62");
          KILL(zBalIterEntry)(&z2zE304);
          KILL(zexception)(&z2zE307);
          goto end_block_exception_1244;
          /* unreachable after throw */
          KILL(zexception)(&z2zE307);
        } else {  z3zE650 = UNIT;  }
        uint64_t z2zE308;
        z2zE308 = zchange.zindex;
        sail_u256 z2zE309;
        z2zE309 = zchange.zvalue;
        {
          z3zE648 = zbal_compare_index_word(zpair, z2zE308, z2zE309);
          if (have_exception) {
            KILL(zBalIterEntry)(&z2zE304);
            goto end_block_exception_1244;
          }
        }
        goto finish_match_1240;
      }
    case_1242: ;
      {
        struct zexception z2zE310;
        CREATE(zexception)(&z2zE310);
        zInvalidBlock(&z2zE310, zInvalidBlockAccessList);
        COPY(zexception)(current_exception, z2zE310);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/executor/block_access_list.sail:157.17-157.59");
        KILL(zBalIterEntry)(&z2zE304);
        KILL(zexception)(&z2zE310);
        goto end_block_exception_1244;
        /* unreachable after throw */
        KILL(zexception)(&z2zE310);
        goto finish_match_1240;
      }
    case_1241: ;
    finish_match_1240: ;
      unit z3zE652;
      z3zE652 = z3zE648;
      KILL(zBalIterEntry)(&z2zE304);
      {
        z3zE647 = zbal_validate_storage_change_values(znext, zslot);
        if (have_exception) {  goto end_block_exception_1244;  }
      }
      goto finish_match_1238;
    }
  case_1239: ;
    sail_match_failure("bal_validate_storage_change_values");
  finish_match_1238: ;
    z8zE671 = z3zE647;
  }
end_function_1243: ;
  return z8zE671;
end_block_exception_1244: ;

  return UNIT;
}

uint64_t zbal_validate_storage_changes(struct zByteSliceFields zcursor, uint64_t zcount, uint64_t zmaximum)
{
  uint64_t z8zE672;
  bool z2zE294;
  {
    uint64_t z2zE293;
    z2zE293 = zcursor.zlen;
    z2zE294 = (z2zE293 == UINT64_C(0));
  }
  if (z2zE294) {  z8zE672 = zcount;  } else {
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE295;
    {
      z2zE295 = zrlp_cursor_pop(zcursor);
      if (have_exception) {  goto end_block_exception_1237;  }
    }
    uint64_t z3zE638;
    {
      struct zRlpFieldRef zslot_field;
      zslot_field = z2zE295.ztup0;
      struct zByteSliceFields znext;
      znext = z2zE295.ztup1;
      struct zByteSliceFields zfields;
      {
        zfields = zbal_ref_cursor(zslot_field);
        if (have_exception) {  goto end_block_exception_1237;  }
      }
      struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE296;
      {
        z2zE296 = zrlp_cursor_pop(zfields);
        if (have_exception) {  goto end_block_exception_1237;  }
      }
      uint64_t z3zE639;
      {
        struct zRlpFieldRef zslot_value;
        zslot_value = z2zE296.ztup0;
        struct zByteSliceFields z1zE11;
        z1zE11 = z2zE296.ztup1;
        struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE297;
        {
          z2zE297 = zrlp_cursor_pop(z1zE11);
          if (have_exception) {  goto end_block_exception_1237;  }
        }
        uint64_t z3zE640;
        {
          struct zRlpFieldRef zchanges_value;
          zchanges_value = z2zE297.ztup0;
          struct zByteSliceFields z1zE12;
          z1zE12 = z2zE297.ztup1;
          unit z3zE641;
          {
            z3zE641 = zbal_expect_end(z1zE12);
            if (have_exception) {  goto end_block_exception_1237;  }
          }
          struct zByteSliceFields zchanges;
          {
            zchanges = zbal_ref_cursor(zchanges_value);
            if (have_exception) {  goto end_block_exception_1237;  }
          }
          bool z2zE299;
          {
            uint64_t z2zE298;
            z2zE298 = zchanges.zlen;
            z2zE299 = (z2zE298 == UINT64_C(0));
          }
          unit z3zE642;
          if (z2zE299) {
            struct zexception z2zE300;
            CREATE(zexception)(&z2zE300);
            zInvalidBlock(&z2zE300, zInvalidBlockAccessList);
            COPY(zexception)(current_exception, z2zE300);
            have_exception = true;
            COPY(sail_string)(throw_location, "sail/executor/block_access_list.sail:182.12-182.54");
            KILL(zexception)(&z2zE300);
            goto end_block_exception_1237;
            /* unreachable after throw */
            KILL(zexception)(&z2zE300);
          } else {  z3zE642 = UNIT;  }
          sail_u256 zslot;
          {
            zslot = zbal_ref_word(zslot_value);
            if (have_exception) {  goto end_block_exception_1237;  }
          }
          unit z3zE643;
          {
            z3zE643 = zbal_validate_storage_change_values(zchanges, zslot);
            if (have_exception) {  goto end_block_exception_1237;  }
          }
          uint64_t z1zE13;
          {
            z1zE13 = zbal_count_item(zcount, zmaximum);
            if (have_exception) {  goto end_block_exception_1237;  }
          }
          {
            z3zE640 = zbal_validate_storage_changes(znext, z1zE13, zmaximum);
            if (have_exception) {  goto end_block_exception_1237;  }
          }
          goto finish_match_1234;
        }
      case_1235: ;
        sail_match_failure("bal_validate_storage_changes");
      finish_match_1234: ;
        z3zE639 = z3zE640;
        goto finish_match_1232;
      }
    case_1233: ;
      sail_match_failure("bal_validate_storage_changes");
    finish_match_1232: ;
      z3zE638 = z3zE639;
      goto finish_match_1230;
    }
  case_1231: ;
    sail_match_failure("bal_validate_storage_changes");
  finish_match_1230: ;
    z8zE672 = z3zE638;
  }
end_function_1236: ;
  return z8zE672;
end_block_exception_1237: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zbal_validate_storage_reads(struct zByteSliceFields zcursor, uint64_t zcount, uint64_t zmaximum)
{
  uint64_t z8zE673;
  bool z2zE287;
  {
    uint64_t z2zE286;
    z2zE286 = zcursor.zlen;
    z2zE287 = (z2zE286 == UINT64_C(0));
  }
  if (z2zE287) {  z8zE673 = zcount;  } else {
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE288;
    {
      z2zE288 = zrlp_cursor_pop(zcursor);
      if (have_exception) {  goto end_block_exception_1229;  }
    }
    uint64_t z3zE632;
    {
      struct zRlpFieldRef zslot_field;
      zslot_field = z2zE288.ztup0;
      struct zByteSliceFields znext;
      znext = z2zE288.ztup1;
      sail_u256 zslot;
      {
        zslot = zbal_ref_word(zslot_field);
        if (have_exception) {  goto end_block_exception_1229;  }
      }
      struct zBalIterEntry z2zE289;
      CREATE(zBalIterEntry)(&z2zE289);
      bal_iter_next(&z2zE289, UNIT);
      unit z3zE633;
      {
        if (z2zE289.kind != Kind_zBalStorageRead) goto case_1227;
        sail_u256 zrecorded;
        zrecorded = z2zE289.variants.zBalStorageRead;
        bool z2zE290;
        z2zE290 = (!eq_u256(zrecorded, zslot));
        if (z2zE290) {
          struct zexception z2zE291;
          CREATE(zexception)(&z2zE291);
          zInvalidBlock(&z2zE291, zInvalidBlockAccessList);
          COPY(zexception)(current_exception, z2zE291);
          have_exception = true;
          COPY(sail_string)(throw_location, "sail/executor/block_access_list.sail:208.20-208.62");
          KILL(zBalIterEntry)(&z2zE289);
          KILL(zexception)(&z2zE291);
          goto end_block_exception_1229;
          /* unreachable after throw */
          KILL(zexception)(&z2zE291);
        } else {  z3zE633 = UNIT;  }
        goto finish_match_1225;
      }
    case_1227: ;
      {
        struct zexception z2zE292;
        CREATE(zexception)(&z2zE292);
        zInvalidBlock(&z2zE292, zInvalidBlockAccessList);
        COPY(zexception)(current_exception, z2zE292);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/executor/block_access_list.sail:211.17-211.59");
        KILL(zBalIterEntry)(&z2zE289);
        KILL(zexception)(&z2zE292);
        goto end_block_exception_1229;
        /* unreachable after throw */
        KILL(zexception)(&z2zE292);
        goto finish_match_1225;
      }
    case_1226: ;
    finish_match_1225: ;
      unit z3zE636;
      z3zE636 = z3zE633;
      KILL(zBalIterEntry)(&z2zE289);
      uint64_t z1zE10;
      {
        z1zE10 = zbal_count_item(zcount, zmaximum);
        if (have_exception) {  goto end_block_exception_1229;  }
      }
      {
        z3zE632 = zbal_validate_storage_reads(znext, z1zE10, zmaximum);
        if (have_exception) {  goto end_block_exception_1229;  }
      }
      goto finish_match_1223;
    }
  case_1224: ;
    sail_match_failure("bal_validate_storage_reads");
  finish_match_1223: ;
    z8zE673 = z3zE632;
  }
end_function_1228: ;
  return z8zE673;
end_block_exception_1229: ;

  return UINT64_C(0xdeadc0de);
}

unit zbal_validate_balance_changes(struct zByteSliceFields zcursor)
{
  unit z8zE674;
  bool z2zE280;
  {
    uint64_t z2zE279;
    z2zE279 = zcursor.zlen;
    z2zE280 = (z2zE279 == UINT64_C(0));
  }
  if (z2zE280) {  z8zE674 = UNIT;  } else {
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE281;
    {
      z2zE281 = zrlp_cursor_pop(zcursor);
      if (have_exception) {  goto end_block_exception_1222;  }
    }
    unit z3zE626;
    {
      struct zRlpFieldRef zpair;
      zpair = z2zE281.ztup0;
      struct zByteSliceFields znext;
      znext = z2zE281.ztup1;
      struct zBalIterEntry z2zE282;
      CREATE(zBalIterEntry)(&z2zE282);
      bal_iter_next(&z2zE282, UNIT);
      unit z3zE627;
      {
        if (z2zE282.kind != Kind_zBalBalanceChange) goto case_1220;
        struct zBalBalanceChangeEntry zchange;
        zchange = z2zE282.variants.zBalBalanceChange;
        uint64_t z2zE283;
        z2zE283 = zchange.zindex;
        sail_u256 z2zE284;
        z2zE284 = zchange.zvalue;
        {
          z3zE627 = zbal_compare_index_word(zpair, z2zE283, z2zE284);
          if (have_exception) {
            KILL(zBalIterEntry)(&z2zE282);
            goto end_block_exception_1222;
          }
        }
        goto finish_match_1218;
      }
    case_1220: ;
      {
        struct zexception z2zE285;
        CREATE(zexception)(&z2zE285);
        zInvalidBlock(&z2zE285, zInvalidBlockAccessList);
        COPY(zexception)(current_exception, z2zE285);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/executor/block_access_list.sail:230.17-230.59");
        KILL(zBalIterEntry)(&z2zE282);
        KILL(zexception)(&z2zE285);
        goto end_block_exception_1222;
        /* unreachable after throw */
        KILL(zexception)(&z2zE285);
        goto finish_match_1218;
      }
    case_1219: ;
    finish_match_1218: ;
      unit z3zE630;
      z3zE630 = z3zE627;
      KILL(zBalIterEntry)(&z2zE282);
      {
        z3zE626 = zbal_validate_balance_changes(znext);
        if (have_exception) {  goto end_block_exception_1222;  }
      }
      goto finish_match_1216;
    }
  case_1217: ;
    sail_match_failure("bal_validate_balance_changes");
  finish_match_1216: ;
    z8zE674 = z3zE626;
  }
end_function_1221: ;
  return z8zE674;
end_block_exception_1222: ;

  return UNIT;
}

unit zbal_validate_nonce_changes(struct zByteSliceFields zcursor)
{
  unit z8zE675;
  bool z2zE273;
  {
    uint64_t z2zE272;
    z2zE272 = zcursor.zlen;
    z2zE273 = (z2zE272 == UINT64_C(0));
  }
  if (z2zE273) {  z8zE675 = UNIT;  } else {
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE274;
    {
      z2zE274 = zrlp_cursor_pop(zcursor);
      if (have_exception) {  goto end_block_exception_1215;  }
    }
    unit z3zE620;
    {
      struct zRlpFieldRef zpair;
      zpair = z2zE274.ztup0;
      struct zByteSliceFields znext;
      znext = z2zE274.ztup1;
      struct zBalIterEntry z2zE275;
      CREATE(zBalIterEntry)(&z2zE275);
      bal_iter_next(&z2zE275, UNIT);
      unit z3zE621;
      {
        if (z2zE275.kind != Kind_zBalNonceChange) goto case_1213;
        struct zBalNonceChangeEntry zchange;
        zchange = z2zE275.variants.zBalNonceChange;
        uint64_t z2zE276;
        z2zE276 = zchange.zindex;
        uint64_t z2zE277;
        z2zE277 = zchange.zvalue;
        {
          z3zE621 = zbal_compare_index_nonce(zpair, z2zE276, z2zE277);
          if (have_exception) {
            KILL(zBalIterEntry)(&z2zE275);
            goto end_block_exception_1215;
          }
        }
        goto finish_match_1211;
      }
    case_1213: ;
      {
        struct zexception z2zE278;
        CREATE(zexception)(&z2zE278);
        zInvalidBlock(&z2zE278, zInvalidBlockAccessList);
        COPY(zexception)(current_exception, z2zE278);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/executor/block_access_list.sail:248.17-248.59");
        KILL(zBalIterEntry)(&z2zE275);
        KILL(zexception)(&z2zE278);
        goto end_block_exception_1215;
        /* unreachable after throw */
        KILL(zexception)(&z2zE278);
        goto finish_match_1211;
      }
    case_1212: ;
    finish_match_1211: ;
      unit z3zE624;
      z3zE624 = z3zE621;
      KILL(zBalIterEntry)(&z2zE275);
      {
        z3zE620 = zbal_validate_nonce_changes(znext);
        if (have_exception) {  goto end_block_exception_1215;  }
      }
      goto finish_match_1209;
    }
  case_1210: ;
    sail_match_failure("bal_validate_nonce_changes");
  finish_match_1209: ;
    z8zE675 = z3zE620;
  }
end_function_1214: ;
  return z8zE675;
end_block_exception_1215: ;

  return UNIT;
}

unit zbal_validate_code_changes(struct zByteSliceFields zcursor)
{
  unit z8zE676;
  bool z2zE266;
  {
    uint64_t z2zE265;
    z2zE265 = zcursor.zlen;
    z2zE266 = (z2zE265 == UINT64_C(0));
  }
  if (z2zE266) {  z8zE676 = UNIT;  } else {
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE267;
    {
      z2zE267 = zrlp_cursor_pop(zcursor);
      if (have_exception) {  goto end_block_exception_1208;  }
    }
    unit z3zE614;
    {
      struct zRlpFieldRef zpair;
      zpair = z2zE267.ztup0;
      struct zByteSliceFields znext;
      znext = z2zE267.ztup1;
      struct zBalIterEntry z2zE268;
      CREATE(zBalIterEntry)(&z2zE268);
      bal_iter_next(&z2zE268, UNIT);
      unit z3zE615;
      {
        if (z2zE268.kind != Kind_zBalCodeChange) goto case_1206;
        struct zBalCodeChangeEntry zchange;
        zchange = z2zE268.variants.zBalCodeChange;
        uint64_t z2zE269;
        z2zE269 = zchange.zindex;
        sail_fixed_bytes_32 z2zE270;
        z2zE270 = zchange.zcode_hash;
        {
          z3zE615 = zbal_compare_index_code(zpair, z2zE269, z2zE270);
          if (have_exception) {
            KILL(zBalIterEntry)(&z2zE268);
            goto end_block_exception_1208;
          }
        }
        goto finish_match_1204;
      }
    case_1206: ;
      {
        struct zexception z2zE271;
        CREATE(zexception)(&z2zE271);
        zInvalidBlock(&z2zE271, zInvalidBlockAccessList);
        COPY(zexception)(current_exception, z2zE271);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/executor/block_access_list.sail:266.17-266.59");
        KILL(zBalIterEntry)(&z2zE268);
        KILL(zexception)(&z2zE271);
        goto end_block_exception_1208;
        /* unreachable after throw */
        KILL(zexception)(&z2zE271);
        goto finish_match_1204;
      }
    case_1205: ;
    finish_match_1204: ;
      unit z3zE618;
      z3zE618 = z3zE615;
      KILL(zBalIterEntry)(&z2zE268);
      {
        z3zE614 = zbal_validate_code_changes(znext);
        if (have_exception) {  goto end_block_exception_1208;  }
      }
      goto finish_match_1202;
    }
  case_1203: ;
    sail_match_failure("bal_validate_code_changes");
  finish_match_1202: ;
    z8zE676 = z3zE614;
  }
end_function_1207: ;
  return z8zE676;
end_block_exception_1208: ;

  return UNIT;
}

uint64_t zbal_validate_accounts(struct zByteSliceFields zcursor, uint64_t zcount, uint64_t zmaximum)
{
  uint64_t z8zE677;
  bool z2zE242;
  {
    uint64_t z2zE241;
    z2zE241 = zcursor.zlen;
    z2zE242 = (z2zE241 == UINT64_C(0));
  }
  if (z2zE242) {  z8zE677 = zcount;  } else {
    struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE243;
    {
      z2zE243 = zrlp_cursor_pop(zcursor);
      if (have_exception) {  goto end_block_exception_1201;  }
    }
    uint64_t z3zE587;
    {
      struct zRlpFieldRef zaccount_field;
      zaccount_field = z2zE243.ztup0;
      struct zByteSliceFields znext;
      znext = z2zE243.ztup1;
      struct zByteSliceFields zfields;
      {
        zfields = zbal_ref_cursor(zaccount_field);
        if (have_exception) {  goto end_block_exception_1201;  }
      }
      struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE244;
      {
        z2zE244 = zrlp_cursor_pop(zfields);
        if (have_exception) {  goto end_block_exception_1201;  }
      }
      uint64_t z3zE588;
      {
        struct zRlpFieldRef zaddress_field;
        zaddress_field = z2zE244.ztup0;
        struct zByteSliceFields z1zE1;
        z1zE1 = z2zE244.ztup1;
        struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE245;
        {
          z2zE245 = zrlp_cursor_pop(z1zE1);
          if (have_exception) {  goto end_block_exception_1201;  }
        }
        uint64_t z3zE589;
        {
          struct zRlpFieldRef zstorage_changes_field;
          zstorage_changes_field = z2zE245.ztup0;
          struct zByteSliceFields z1zE2;
          z1zE2 = z2zE245.ztup1;
          struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE246;
          {
            z2zE246 = zrlp_cursor_pop(z1zE2);
            if (have_exception) {  goto end_block_exception_1201;  }
          }
          uint64_t z3zE590;
          {
            struct zRlpFieldRef zstorage_reads_field;
            zstorage_reads_field = z2zE246.ztup0;
            struct zByteSliceFields z1zE3;
            z1zE3 = z2zE246.ztup1;
            struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE247;
            {
              z2zE247 = zrlp_cursor_pop(z1zE3);
              if (have_exception) {  goto end_block_exception_1201;  }
            }
            uint64_t z3zE591;
            {
              struct zRlpFieldRef zbalance_changes_field;
              zbalance_changes_field = z2zE247.ztup0;
              struct zByteSliceFields z1zE4;
              z1zE4 = z2zE247.ztup1;
              struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE248;
              {
                z2zE248 = zrlp_cursor_pop(z1zE4);
                if (have_exception) {  goto end_block_exception_1201;  }
              }
              uint64_t z3zE592;
              {
                struct zRlpFieldRef znonce_changes_field;
                znonce_changes_field = z2zE248.ztup0;
                struct zByteSliceFields z1zE5;
                z1zE5 = z2zE248.ztup1;
                struct ztuple_z8z5structz0zzRlpFieldRefzCz0z5structz0zzByteSliceFieldsz9 z2zE249;
                {
                  z2zE249 = zrlp_cursor_pop(z1zE5);
                  if (have_exception) {  goto end_block_exception_1201;  }
                }
                uint64_t z3zE593;
                {
                  struct zRlpFieldRef zcode_changes_field;
                  zcode_changes_field = z2zE249.ztup0;
                  struct zByteSliceFields z1zE6;
                  z1zE6 = z2zE249.ztup1;
                  unit z3zE594;
                  {
                    z3zE594 = zbal_expect_end(z1zE6);
                    if (have_exception) {  goto end_block_exception_1201;  }
                  }
                  struct zByteSliceFields zaddress_bytes;
                  {
                    zaddress_bytes = zbal_ref_bytes(zaddress_field);
                    if (have_exception) {  goto end_block_exception_1201;  }
                  }
                  sail_fixed_bytes_20 zaccount;
                  {
                    sail_u256 z2zE264;
                    {
                      z2zE264 = zrlp_ref_word(zaddress_field);
                      if (have_exception) {  goto end_block_exception_1201;  }
                    }
                    zaccount = evmsail_word_to_address(z2zE264);
                  }
                  bool z2zE251;
                  {
                    uint64_t z2zE250;
                    z2zE250 = zaddress_bytes.zlen;
                    z2zE251 = (z2zE250 != UINT64_C(20));
                  }
                  unit z3zE599;
                  if (z2zE251) {
                    struct zexception z2zE252;
                    CREATE(zexception)(&z2zE252);
                    zInvalidBlock(&z2zE252, zInvalidBlockAccessList);
                    COPY(zexception)(current_exception, z2zE252);
                    have_exception = true;
                    COPY(sail_string)(throw_location, "sail/executor/block_access_list.sail:299.12-299.54");
                    KILL(zexception)(&z2zE252);
                    goto end_block_exception_1201;
                    /* unreachable after throw */
                    KILL(zexception)(&z2zE252);
                  } else {  z3zE599 = UNIT;  }
                  struct zBalIterEntry z2zE253;
                  CREATE(zBalIterEntry)(&z2zE253);
                  bal_iter_next(&z2zE253, UNIT);
                  unit z3zE595;
                  {
                    if (z2zE253.kind != Kind_zBalAccount) goto case_1196;
                    sail_fixed_bytes_20 zrecorded;
                    zrecorded = z2zE253.variants.zBalAccount;
                    bool z2zE254;
                    z2zE254 = zneq_anythingzIR__sail_c_repr_fixed_byteszIC20zKzK(zrecorded, zaccount);
                    if (z2zE254) {
                      struct zexception z2zE255;
                      CREATE(zexception)(&z2zE255);
                      zInvalidBlock(&z2zE255, zInvalidBlockAccessList);
                      COPY(zexception)(current_exception, z2zE255);
                      have_exception = true;
                      COPY(sail_string)(throw_location, "sail/executor/block_access_list.sail:303.16-303.58");
                      KILL(zBalIterEntry)(&z2zE253);
                      KILL(zexception)(&z2zE255);
                      goto end_block_exception_1201;
                      /* unreachable after throw */
                      KILL(zexception)(&z2zE255);
                    } else {  z3zE595 = UNIT;  }
                    goto finish_match_1194;
                  }
                case_1196: ;
                  {
                    struct zexception z2zE256;
                    CREATE(zexception)(&z2zE256);
                    zInvalidBlock(&z2zE256, zInvalidBlockAccessList);
                    COPY(zexception)(current_exception, z2zE256);
                    have_exception = true;
                    COPY(sail_string)(throw_location, "sail/executor/block_access_list.sail:305.17-305.59");
                    KILL(zBalIterEntry)(&z2zE253);
                    KILL(zexception)(&z2zE256);
                    goto end_block_exception_1201;
                    /* unreachable after throw */
                    KILL(zexception)(&z2zE256);
                    goto finish_match_1194;
                  }
                case_1195: ;
                finish_match_1194: ;
                  unit z3zE598;
                  z3zE598 = z3zE595;
                  KILL(zBalIterEntry)(&z2zE253);
                  uint64_t z1zE7;
                  {
                    z1zE7 = zbal_count_item(zcount, zmaximum);
                    if (have_exception) {  goto end_block_exception_1201;  }
                  }
                  uint64_t z1zE8;
                  {
                    struct zByteSliceFields z2zE263;
                    {
                      z2zE263 = zbal_ref_cursor(zstorage_changes_field);
                      if (have_exception) {  goto end_block_exception_1201;  }
                    }
                    {
                      z1zE8 = zbal_validate_storage_changes(z2zE263, z1zE7, zmaximum);
                      if (have_exception) {  goto end_block_exception_1201;  }
                    }
                  }
                  uint64_t z1zE9;
                  {
                    struct zByteSliceFields z2zE262;
                    {
                      z2zE262 = zbal_ref_cursor(zstorage_reads_field);
                      if (have_exception) {  goto end_block_exception_1201;  }
                    }
                    {
                      z1zE9 = zbal_validate_storage_reads(z2zE262, z1zE8, zmaximum);
                      if (have_exception) {  goto end_block_exception_1201;  }
                    }
                  }
                  struct zByteSliceFields z2zE257;
                  {
                    z2zE257 = zbal_ref_cursor(zbalance_changes_field);
                    if (have_exception) {  goto end_block_exception_1201;  }
                  }
                  unit z3zE606;
                  {
                    z3zE606 = zbal_validate_balance_changes(z2zE257);
                    if (have_exception) {  goto end_block_exception_1201;  }
                  }
                  struct zByteSliceFields z2zE258;
                  {
                    z2zE258 = zbal_ref_cursor(znonce_changes_field);
                    if (have_exception) {  goto end_block_exception_1201;  }
                  }
                  unit z3zE605;
                  {
                    z3zE605 = zbal_validate_nonce_changes(z2zE258);
                    if (have_exception) {  goto end_block_exception_1201;  }
                  }
                  struct zByteSliceFields z2zE259;
                  {
                    z2zE259 = zbal_ref_cursor(zcode_changes_field);
                    if (have_exception) {  goto end_block_exception_1201;  }
                  }
                  unit z3zE604;
                  {
                    z3zE604 = zbal_validate_code_changes(z2zE259);
                    if (have_exception) {  goto end_block_exception_1201;  }
                  }
                  struct zBalIterEntry z2zE260;
                  CREATE(zBalIterEntry)(&z2zE260);
                  bal_iter_next(&z2zE260, UNIT);
                  unit z3zE600;
                  {
                    if (z2zE260.kind != Kind_zBalAccountEnd) goto case_1199;
                    z3zE600 = UNIT;
                    goto finish_match_1197;
                  }
                case_1199: ;
                  {
                    struct zexception z2zE261;
                    CREATE(zexception)(&z2zE261);
                    zInvalidBlock(&z2zE261, zInvalidBlockAccessList);
                    COPY(zexception)(current_exception, z2zE261);
                    have_exception = true;
                    COPY(sail_string)(throw_location, "sail/executor/block_access_list.sail:316.17-316.59");
                    KILL(zBalIterEntry)(&z2zE260);
                    KILL(zexception)(&z2zE261);
                    goto end_block_exception_1201;
                    /* unreachable after throw */
                    KILL(zexception)(&z2zE261);
                    goto finish_match_1197;
                  }
                case_1198: ;
                finish_match_1197: ;
                  unit z3zE603;
                  z3zE603 = z3zE600;
                  KILL(zBalIterEntry)(&z2zE260);
                  {
                    z3zE593 = zbal_validate_accounts(znext, z1zE9, zmaximum);
                    if (have_exception) {  goto end_block_exception_1201;  }
                  }
                  goto finish_match_1192;
                }
              case_1193: ;
                sail_match_failure("bal_validate_accounts");
              finish_match_1192: ;
                z3zE592 = z3zE593;
                goto finish_match_1190;
              }
            case_1191: ;
              sail_match_failure("bal_validate_accounts");
            finish_match_1190: ;
              z3zE591 = z3zE592;
              goto finish_match_1188;
            }
          case_1189: ;
            sail_match_failure("bal_validate_accounts");
          finish_match_1188: ;
            z3zE590 = z3zE591;
            goto finish_match_1186;
          }
        case_1187: ;
          sail_match_failure("bal_validate_accounts");
        finish_match_1186: ;
          z3zE589 = z3zE590;
          goto finish_match_1184;
        }
      case_1185: ;
        sail_match_failure("bal_validate_accounts");
      finish_match_1184: ;
        z3zE588 = z3zE589;
        goto finish_match_1182;
      }
    case_1183: ;
      sail_match_failure("bal_validate_accounts");
    finish_match_1182: ;
      z3zE587 = z3zE588;
      goto finish_match_1180;
    }
  case_1181: ;
    sail_match_failure("bal_validate_accounts");
  finish_match_1180: ;
    z8zE677 = z3zE587;
  }
end_function_1200: ;
  return z8zE677;
end_block_exception_1201: ;

  return UINT64_C(0xdeadc0de);
}

unit zvalidate_block_access_list(struct zByteSliceFields zbytes, uint64_t zmaximum_items)
{
  unit z8zE678;
  unit z3zE581;
  z3zE581 = bal_prepare_iter(UNIT);
  struct zRlpFieldRef zroot;
  {
    zroot = zrlp_single_ref(zbytes);
    if (have_exception) {  goto end_block_exception_1179;  }
  }
  uint64_t z2zE238;
  {
    struct zByteSliceFields z2zE237;
    {
      z2zE237 = zbal_ref_cursor(zroot);
      if (have_exception) {  goto end_block_exception_1179;  }
    }
    {
      z2zE238 = zbal_validate_accounts(z2zE237, UINT64_C(0), zmaximum_items);
      if (have_exception) {  goto end_block_exception_1179;  }
    }
  }
  unit z3zE582;
  {
    struct zBalIterEntry z2zE239;
    CREATE(zBalIterEntry)(&z2zE239);
    bal_iter_next(&z2zE239, UNIT);
    unit z3zE583;
    {
      if (z2zE239.kind != Kind_zBalEmpty) goto case_1177;
      z3zE583 = UNIT;
      goto finish_match_1175;
    }
  case_1177: ;
    {
      struct zexception z2zE240;
      CREATE(zexception)(&z2zE240);
      zInvalidBlock(&z2zE240, zInvalidBlockAccessList);
      COPY(zexception)(current_exception, z2zE240);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/executor/block_access_list.sail:330.13-330.55");
      KILL(zBalIterEntry)(&z2zE239);
      KILL(zexception)(&z2zE240);
      goto end_block_exception_1179;
      /* unreachable after throw */
      KILL(zexception)(&z2zE240);
      goto finish_match_1175;
    }
  case_1176: ;
  finish_match_1175: ;
    z3zE582 = z3zE583;
    KILL(zBalIterEntry)(&z2zE239);
    goto finish_match_1173;
  }
case_1174: ;
  sail_match_failure("validate_block_access_list");
finish_match_1173: ;
  z8zE678 = z3zE582;
end_function_1178: ;
  return z8zE678;
end_block_exception_1179: ;

  return UNIT;
}

