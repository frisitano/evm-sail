/* Generated from sail/executor/block_access_list.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void create_letbind_310(void) {

  uint64_t z3zE368;
  z3zE368 = UINT64_C(0);
  zBAL_RLP_ZERO = z3zE368;
let_end_961: ;
}
void kill_letbind_310(void) {
}

uint64_t zbal_rlp_length_to_byte_length(uint64_t zvalue)
{
  uint64_t z8zE620;
  z8zE620 = zvalue;
end_function_1473: ;
  return z8zE620;
end_block_exception_1474: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zbal_rlp_list_sizze(uint64_t zcontent_len)
{
  uint64_t z8zE621;
  uint64_t zcontent_length;
  zcontent_length = zbal_rlp_length_to_byte_length(zcontent_len);
  uint64_t zencoded_length;
  zencoded_length = zrlp_list_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zcontent_length);
  {
    z8zE621 = zbal_rlp_length_from_byte_lengthzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zencoded_length);
    if (have_exception) {  goto end_block_exception_1468;  }
  }
end_function_1467: ;
  return z8zE621;
end_block_exception_1468: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zbal_previous_index(uint64_t zvalue)
{
  uint64_t z8zE622;
  bool z2zE691;
  z2zE691 = (zvalue != UINT64_C(0));
  if (z2zE691) {
    {    z8zE622 = (zvalue - UINT64_C(1));
    }
  } else {
    struct zexception z2zE692;
    CREATE(zexception)(&z2zE692);
    zInvalidBlock(&z2zE692, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE692);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/block_access_list.sail:116.8-116.44");
    KILL(zexception)(&z2zE692);
    goto end_block_exception_1462;
    /* unreachable after throw */
    KILL(zexception)(&z2zE692);
  }
end_function_1461: ;
  return z8zE622;
end_block_exception_1462: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zbal_index_word_content_sizze(uint64_t zindex, sail_u256 zvalue)
{
  uint64_t z8zE623;
  uint64_t z2zE689;
  z2zE689 = zrlp_uint_word_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zindex);
  uint64_t z2zE690;
  z2zE690 = zrlp_uint_word_sizze(zvalue);
  {
    z8zE623 = zbal_bounded_byte_length_addzIreprzGU64zCU64zCU64zKzIboundszGa3b2b826adddb1d18c77e659efbf6001zK(z2zE689, z2zE690);
    if (have_exception) {  goto end_block_exception_1460;  }
  }
end_function_1459: ;
  return z8zE623;
end_block_exception_1460: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zbal_index_word_sizze(uint64_t zindex, sail_u256 zvalue)
{
  uint64_t z8zE624;
  uint64_t z2zE688;
  {
    z2zE688 = zbal_index_word_content_sizze(zindex, zvalue);
    if (have_exception) {  goto end_block_exception_1458;  }
  }
  {
    z8zE624 = zbal_bounded_list_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE688);
    if (have_exception) {  goto end_block_exception_1458;  }
  }
end_function_1457: ;
  return z8zE624;
end_block_exception_1458: ;

  return UINT64_C(0xdeadc0de);
}

unit zbal_write_index_word(uint64_t zindex, sail_u256 zvalue)
{
  unit z8zE625;
  uint64_t z2zE687;
  {
    z2zE687 = zbal_index_word_content_sizze(zindex, zvalue);
    if (have_exception) {  goto end_block_exception_1456;  }
  }
  unit z3zE961;
  z3zE961 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE687);
  unit z3zE960;
  z3zE960 = zrlp_write_uint_wordzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zindex);
  z8zE625 = zrlp_write_uint_word(zvalue);
end_function_1455: ;
  return z8zE625;
end_block_exception_1456: ;

  return UNIT;
}

uint64_t zbal_index_nonce_content_sizze(uint64_t zindex, uint64_t zvalue)
{
  uint64_t z8zE626;
  uint64_t z2zE685;
  z2zE685 = zrlp_uint_word_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zindex);
  uint64_t z2zE686;
  z2zE686 = zrlp_uint_word_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zvalue);
  {
    z8zE626 = zbal_bounded_byte_length_addzIreprzGU64zCU64zCU64zKzIboundszGa3b2b826adddb1d18c77e659efbf6001zK(z2zE685, z2zE686);
    if (have_exception) {  goto end_block_exception_1454;  }
  }
end_function_1453: ;
  return z8zE626;
end_block_exception_1454: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zbal_index_nonce_sizze(uint64_t zindex, uint64_t zvalue)
{
  uint64_t z8zE627;
  uint64_t z2zE684;
  {
    z2zE684 = zbal_index_nonce_content_sizze(zindex, zvalue);
    if (have_exception) {  goto end_block_exception_1452;  }
  }
  {
    z8zE627 = zbal_bounded_list_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE684);
    if (have_exception) {  goto end_block_exception_1452;  }
  }
end_function_1451: ;
  return z8zE627;
end_block_exception_1452: ;

  return UINT64_C(0xdeadc0de);
}

unit zbal_write_index_nonce(uint64_t zindex, uint64_t zvalue)
{
  unit z8zE628;
  uint64_t z2zE683;
  {
    z2zE683 = zbal_index_nonce_content_sizze(zindex, zvalue);
    if (have_exception) {  goto end_block_exception_1450;  }
  }
  unit z3zE959;
  z3zE959 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE683);
  unit z3zE958;
  z3zE958 = zrlp_write_uint_wordzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zindex);
  z8zE628 = zrlp_write_uint_wordzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zvalue);
end_function_1449: ;
  return z8zE628;
end_block_exception_1450: ;

  return UNIT;
}

uint64_t zbal_index_code_content_sizze(uint64_t zindex, sail_fixed_bytes_32 zcode_hash)
{
  uint64_t z8zE629;
  struct zCode zcode;
  {
    zcode = zcode_db_resolve(zcode_hash);
    if (have_exception) {  goto end_block_exception_1448;  }
  }
  uint64_t zindex_length;
  zindex_length = zrlp_uint_word_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zindex);
  sail_u128 zcode_length;
  {
    struct zByteSliceFields z2zE682;
    z2zE682 = zcode.zbytes;
    zcode_length = zrlp_slice_sizze(z2zE682);
  }
  {
    z8zE629 = zbal_bounded_byte_length_addzIreprzGU64zCR__sail_c_repr_u128zCU64zKzIboundszG69b6fbdbe405b82ee0c43387d0dda91azK(zindex_length, zcode_length);
    if (have_exception) {  goto end_block_exception_1448;  }
  }
end_function_1447: ;
  return z8zE629;
end_block_exception_1448: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zbal_index_code_sizze(uint64_t zindex, sail_fixed_bytes_32 zcode_hash)
{
  uint64_t z8zE630;
  uint64_t z2zE681;
  {
    z2zE681 = zbal_index_code_content_sizze(zindex, zcode_hash);
    if (have_exception) {  goto end_block_exception_1446;  }
  }
  {
    z8zE630 = zbal_bounded_list_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE681);
    if (have_exception) {  goto end_block_exception_1446;  }
  }
end_function_1445: ;
  return z8zE630;
end_block_exception_1446: ;

  return UINT64_C(0xdeadc0de);
}

unit zbal_write_index_code(uint64_t zindex, sail_fixed_bytes_32 zcode_hash)
{
  unit z8zE631;
  struct zCode zcode;
  {
    zcode = zcode_db_resolve(zcode_hash);
    if (have_exception) {  goto end_block_exception_1444;  }
  }
  uint64_t z2zE679;
  {
    z2zE679 = zbal_index_code_content_sizze(zindex, zcode_hash);
    if (have_exception) {  goto end_block_exception_1444;  }
  }
  unit z3zE957;
  z3zE957 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE679);
  unit z3zE956;
  z3zE956 = zrlp_write_uint_wordzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zindex);
  struct zByteSliceFields z2zE680;
  z2zE680 = zcode.zbytes;
  z8zE631 = zrlp_write_slice(z2zE680);
end_function_1443: ;
  return z8zE631;
end_block_exception_1444: ;

  return UNIT;
}

uint64_t zbal_storage_change_run_end(uint64_t zaccount, uint64_t zcount, sail_u256 zslot, uint64_t zindex, uint64_t zcursor)
{
  uint64_t z8zE632;
  bool z2zE673;
  z2zE673 = (zcursor < zcount);
  if (z2zE673) {
    bool z2zE677;
    {
      bool z2zE676;
      {
        sail_u256 z2zE674;
        z2zE674 = bal_storage_change_slot(zaccount, zcursor);
        z2zE676 = eq_u256(z2zE674, zslot);
      }
      bool z3zE955;
      if (z2zE676) {
        uint64_t z2zE675;
        z2zE675 = bal_storage_change_index(zaccount, zcursor);
        z3zE955 = (z2zE675 == zindex);
      } else {  z3zE955 = false;  }
      z2zE677 = z3zE955;
    }
    if (z2zE677) {
      uint64_t z2zE678;
      {    z2zE678 = (zcursor + UINT64_C(1));
      }
      z8zE632 = zbal_storage_change_run_end(zaccount, zcount, zslot, zindex, z2zE678);
    } else {  z8zE632 = zcursor;  }
  } else {  z8zE632 = zcursor;  }
end_function_1441: ;
  return z8zE632;
end_block_exception_1442: ;

  return UINT64_C(0xdeadc0de);
}

struct zBalContentCursor zbal_storage_slot_changes_sizze(uint64_t zaccount, uint64_t zcount, sail_u256 zslot, uint64_t zcursor, uint64_t zcontent_len)
{
  struct zBalContentCursor z8zE633;
  bool z2zE668;
  z2zE668 = (zcursor < zcount);
  if (z2zE668) {
    bool z2zE670;
    {
      sail_u256 z2zE669;
      z2zE669 = bal_storage_change_slot(zaccount, zcursor);
      z2zE670 = eq_u256(z2zE669, zslot);
    }
    if (z2zE670) {
      uint64_t zindex;
      zindex = bal_storage_change_index(zaccount, zcursor);
      uint64_t znext;
      znext = zbal_storage_change_run_end(zaccount, zcount, zslot, zindex, zcursor);
      uint64_t zlast;
      {
        zlast = zbal_previous_index(znext);
        if (have_exception) {  goto end_block_exception_1440;  }
      }
      uint64_t znext_len;
      {
        uint64_t z2zE672;
        {
          sail_u256 z2zE671;
          z2zE671 = bal_storage_change_value(zaccount, zlast);
          {
            z2zE672 = zbal_index_word_sizze(zindex, z2zE671);
            if (have_exception) {  goto end_block_exception_1440;  }
          }
        }
        {
          znext_len = zbal_rlp_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_len, z2zE672);
          if (have_exception) {  goto end_block_exception_1440;  }
        }
      }
      {
        z8zE633 = zbal_storage_slot_changes_sizze(zaccount, zcount, zslot, znext, znext_len);
        if (have_exception) {  goto end_block_exception_1440;  }
      }
    } else {
      struct zBalContentCursor z3zE954;
      z3zE954.zcontent_len = zcontent_len;
      z3zE954.zcursor = zcursor;
      z8zE633 = z3zE954;
    }
  } else {
    struct zBalContentCursor z3zE953;
    z3zE953.zcontent_len = zcontent_len;
    z3zE953.zcursor = zcursor;
    z8zE633 = z3zE953;
  }
end_function_1439: ;
  return z8zE633;
end_block_exception_1440: ;
  struct zBalContentCursor z8zE1101 = { .zcontent_len = UINT64_C(0xdeadc0de), .zcursor = UINT64_C(0xdeadc0de) };
  return z8zE1101;
}

uint64_t zbal_write_storage_slot_changes(uint64_t zaccount, uint64_t zcount, sail_u256 zslot, uint64_t zcursor)
{
  uint64_t z8zE634;
  bool z2zE664;
  z2zE664 = (zcursor < zcount);
  if (z2zE664) {
    bool z2zE666;
    {
      sail_u256 z2zE665;
      z2zE665 = bal_storage_change_slot(zaccount, zcursor);
      z2zE666 = eq_u256(z2zE665, zslot);
    }
    if (z2zE666) {
      uint64_t zindex;
      zindex = bal_storage_change_index(zaccount, zcursor);
      uint64_t znext;
      znext = zbal_storage_change_run_end(zaccount, zcount, zslot, zindex, zcursor);
      uint64_t zlast;
      {
        zlast = zbal_previous_index(znext);
        if (have_exception) {  goto end_block_exception_1438;  }
      }
      sail_u256 z2zE667;
      z2zE667 = bal_storage_change_value(zaccount, zlast);
      unit z3zE952;
      {
        z3zE952 = zbal_write_index_word(zindex, z2zE667);
        if (have_exception) {  goto end_block_exception_1438;  }
      }
      {
        z8zE634 = zbal_write_storage_slot_changes(zaccount, zcount, zslot, znext);
        if (have_exception) {  goto end_block_exception_1438;  }
      }
    } else {  z8zE634 = zcursor;  }
  } else {  z8zE634 = zcursor;  }
end_function_1437: ;
  return z8zE634;
end_block_exception_1438: ;

  return UINT64_C(0xdeadc0de);
}

struct zBalContentCount zbal_storage_change_groups_sizze(uint64_t zaccount, uint64_t zcount, uint64_t zcursor, struct zBalContentCount zresult)
{
  struct zBalContentCount z8zE635;
  bool z2zE651;
  z2zE651 = (zcursor < zcount);
  if (z2zE651) {
    sail_u256 zslot;
    zslot = bal_storage_change_slot(zaccount, zcursor);
    struct zBalContentCursor zchanges;
    {
      zchanges = zbal_storage_slot_changes_sizze(zaccount, zcount, zslot, zcursor, zBAL_RLP_ZERO);
      if (have_exception) {  goto end_block_exception_1436;  }
    }
    uint64_t zslot_content_len;
    {
      uint64_t z2zE662;
      {
        uint64_t z2zE659;
        z2zE659 = zrlp_uint_word_sizze(zslot);
        {
          z2zE662 = zbal_rlp_length_from_byte_lengthzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE659);
          if (have_exception) {  goto end_block_exception_1436;  }
        }
      }
      uint64_t z2zE663;
      {
        uint64_t z2zE661;
        {
          uint64_t z2zE660;
          z2zE660 = zchanges.zcontent_len;
          {
            z2zE661 = zbal_rlp_list_sizze(z2zE660);
            if (have_exception) {  goto end_block_exception_1436;  }
          }
        }
        z2zE663 = zbal_rlp_length_to_byte_length(z2zE661);
      }
      {
        zslot_content_len = zbal_rlp_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE662, z2zE663);
        if (have_exception) {  goto end_block_exception_1436;  }
      }
    }
    uint64_t znext_content_len;
    {
      uint64_t z2zE657;
      z2zE657 = zresult.zcontent_len;
      uint64_t z2zE658;
      {
        uint64_t z2zE656;
        {
          z2zE656 = zbal_rlp_list_sizze(zslot_content_len);
          if (have_exception) {  goto end_block_exception_1436;  }
        }
        z2zE658 = zbal_rlp_length_to_byte_length(z2zE656);
      }
      {
        znext_content_len = zbal_rlp_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE657, z2zE658);
        if (have_exception) {  goto end_block_exception_1436;  }
      }
    }
    uint64_t z2zE654;
    z2zE654 = zchanges.zcursor;
    struct zBalContentCount z2zE655;
    {
      uint64_t z2zE653;
      {
        uint64_t z2zE652;
        z2zE652 = zresult.zcount;
        {
          z2zE653 = zbal_count_addzIreprzGU64zCU64zCU64zKzIboundszGd0328719294598864996af0723f2d8e9zK(z2zE652, UINT64_C(1));
          if (have_exception) {  goto end_block_exception_1436;  }
        }
      }
      struct zBalContentCount z3zE951;
      z3zE951.zcontent_len = znext_content_len;
      z3zE951.zcount = z2zE653;
      z2zE655 = z3zE951;
    }
    {
      z8zE635 = zbal_storage_change_groups_sizze(zaccount, zcount, z2zE654, z2zE655);
      if (have_exception) {  goto end_block_exception_1436;  }
    }
  } else {  z8zE635 = zresult;  }
end_function_1435: ;
  return z8zE635;
end_block_exception_1436: ;
  struct zBalContentCount z8zE1102 = { .zcontent_len = UINT64_C(0xdeadc0de), .zcount = UINT64_C(0xdeadc0de) };
  return z8zE1102;
}

unit zbal_write_storage_change_groups(uint64_t zaccount, uint64_t zcount, uint64_t zcursor)
{
  unit z8zE636;
  bool z2zE640;
  z2zE640 = (zcursor < zcount);
  if (z2zE640) {
    sail_u256 zslot;
    zslot = bal_storage_change_slot(zaccount, zcursor);
    struct zBalContentCursor zchanges;
    {
      zchanges = zbal_storage_slot_changes_sizze(zaccount, zcount, zslot, zcursor, zBAL_RLP_ZERO);
      if (have_exception) {  goto end_block_exception_1434;  }
    }
    uint64_t zslot_content_len;
    {
      uint64_t z2zE649;
      {
        uint64_t z2zE646;
        z2zE646 = zrlp_uint_word_sizze(zslot);
        {
          z2zE649 = zbal_rlp_length_from_byte_lengthzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE646);
          if (have_exception) {  goto end_block_exception_1434;  }
        }
      }
      uint64_t z2zE650;
      {
        uint64_t z2zE648;
        {
          uint64_t z2zE647;
          z2zE647 = zchanges.zcontent_len;
          {
            z2zE648 = zbal_rlp_list_sizze(z2zE647);
            if (have_exception) {  goto end_block_exception_1434;  }
          }
        }
        z2zE650 = zbal_rlp_length_to_byte_length(z2zE648);
      }
      {
        zslot_content_len = zbal_rlp_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE649, z2zE650);
        if (have_exception) {  goto end_block_exception_1434;  }
      }
    }
    uint64_t z2zE641;
    z2zE641 = zbal_rlp_length_to_byte_length(zslot_content_len);
    unit z3zE949;
    z3zE949 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE641);
    unit z3zE948;
    z3zE948 = zrlp_write_uint_word(zslot);
    uint64_t z2zE643;
    {
      uint64_t z2zE642;
      z2zE642 = zchanges.zcontent_len;
      z2zE643 = zbal_rlp_length_to_byte_length(z2zE642);
    }
    unit z3zE947;
    z3zE947 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE643);
    uint64_t znext;
    {
      znext = zbal_write_storage_slot_changes(zaccount, zcount, zslot, zcursor);
      if (have_exception) {  goto end_block_exception_1434;  }
    }
    bool z2zE645;
    {
      uint64_t z2zE644;
      z2zE644 = zchanges.zcursor;
      z2zE645 = (znext == z2zE644);
    }
    unit z3zE950;
    z3zE950 = sail_assert(z2zE645, "BAL storage-change sizing cursor");
    {
      z8zE636 = zbal_write_storage_change_groups(zaccount, zcount, znext);
      if (have_exception) {  goto end_block_exception_1434;  }
    }
  } else {  z8zE636 = UNIT;  }
end_function_1433: ;
  return z8zE636;
end_block_exception_1434: ;

  return UNIT;
}

struct zBalContentCount zbal_storage_changes_sizze(uint64_t zaccount)
{
  struct zBalContentCount z8zE637;
  uint64_t z2zE639;
  z2zE639 = bal_storage_change_count(zaccount);
  struct zBalContentCount z3zE946;
  z3zE946.zcontent_len = zBAL_RLP_ZERO;
  z3zE946.zcount = UINT64_C(0);
  {
    z8zE637 = zbal_storage_change_groups_sizze(zaccount, z2zE639, UINT64_C(0), z3zE946);
    if (have_exception) {  goto end_block_exception_1432;  }
  }
end_function_1431: ;
  return z8zE637;
end_block_exception_1432: ;
  struct zBalContentCount z8zE1103 = { .zcontent_len = UINT64_C(0xdeadc0de), .zcount = UINT64_C(0xdeadc0de) };
  return z8zE1103;
}

unit zbal_write_storage_changes(uint64_t zaccount, struct zBalContentCount zsizze)
{
  unit z8zE638;
  uint64_t z2zE637;
  {
    uint64_t z2zE636;
    z2zE636 = zsizze.zcontent_len;
    z2zE637 = zbal_rlp_length_to_byte_length(z2zE636);
  }
  unit z3zE945;
  z3zE945 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE637);
  uint64_t z2zE638;
  z2zE638 = bal_storage_change_count(zaccount);
  {
    z8zE638 = zbal_write_storage_change_groups(zaccount, z2zE638, UINT64_C(0));
    if (have_exception) {  goto end_block_exception_1430;  }
  }
end_function_1429: ;
  return z8zE638;
end_block_exception_1430: ;

  return UNIT;
}

uint64_t zbal_storage_read_run_end(uint64_t zaccount, uint64_t zcount, sail_u256 zslot, uint64_t zcursor)
{
  uint64_t z8zE639;
  bool z2zE632;
  z2zE632 = (zcursor < zcount);
  if (z2zE632) {
    bool z2zE634;
    {
      sail_u256 z2zE633;
      z2zE633 = bal_storage_read_slot(zaccount, zcursor);
      z2zE634 = eq_u256(z2zE633, zslot);
    }
    if (z2zE634) {
      uint64_t z2zE635;
      {    z2zE635 = (zcursor + UINT64_C(1));
      }
      z8zE639 = zbal_storage_read_run_end(zaccount, zcount, zslot, z2zE635);
    } else {  z8zE639 = zcursor;  }
  } else {  z8zE639 = zcursor;  }
end_function_1427: ;
  return z8zE639;
end_block_exception_1428: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zbal_storage_change_seek(uint64_t zaccount, uint64_t zcount, sail_u256 zslot, uint64_t zcursor)
{
  uint64_t z8zE640;
  bool z2zE628;
  z2zE628 = (zcursor < zcount);
  if (z2zE628) {
    bool z2zE630;
    {
      sail_u256 z2zE629;
      z2zE629 = bal_storage_change_slot(zaccount, zcursor);
      z2zE630 = zword_ult(z2zE629, zslot);
    }
    if (z2zE630) {
      uint64_t z2zE631;
      {    z2zE631 = (zcursor + UINT64_C(1));
      }
      z8zE640 = zbal_storage_change_seek(zaccount, zcount, zslot, z2zE631);
    } else {  z8zE640 = zcursor;  }
  } else {  z8zE640 = zcursor;  }
end_function_1425: ;
  return z8zE640;
end_block_exception_1426: ;

  return UINT64_C(0xdeadc0de);
}

struct zBalContentCount zbal_storage_read_groups_sizze(uint64_t zaccount, uint64_t zread_count, uint64_t zchange_count, uint64_t zread, uint64_t zchange, struct zBalContentCount zresult)
{
  struct zBalContentCount z8zE641;
  bool z2zE620;
  z2zE620 = (zread < zread_count);
  if (z2zE620) {
    sail_u256 zslot;
    zslot = bal_storage_read_slot(zaccount, zread);
    uint64_t znext_read;
    znext_read = zbal_storage_read_run_end(zaccount, zread_count, zslot, zread);
    uint64_t znext_change;
    znext_change = zbal_storage_change_seek(zaccount, zchange_count, zslot, zchange);
    bool zchanged;
    {
      bool z2zE626;
      z2zE626 = (znext_change < zchange_count);
      if (z2zE626) {
        sail_u256 z2zE627;
        z2zE627 = bal_storage_change_slot(zaccount, znext_change);
        zchanged = eq_u256(z2zE627, zslot);
      } else {  zchanged = false;  }
    }
    struct zBalContentCount zupdated;
    if (zchanged) {  zupdated = zresult;  } else {
      uint64_t z2zE623;
      {
        uint64_t z2zE621;
        z2zE621 = zresult.zcontent_len;
        uint64_t z2zE622;
        z2zE622 = zrlp_uint_word_sizze(zslot);
        {
          z2zE623 = zbal_rlp_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE621, z2zE622);
          if (have_exception) {  goto end_block_exception_1424;  }
        }
      }
      uint64_t z2zE625;
      {
        uint64_t z2zE624;
        z2zE624 = zresult.zcount;
        {
          z2zE625 = zbal_count_addzIreprzGU64zCU64zCU64zKzIboundszGd0328719294598864996af0723f2d8e9zK(z2zE624, UINT64_C(1));
          if (have_exception) {  goto end_block_exception_1424;  }
        }
      }
      struct zBalContentCount z3zE944;
      z3zE944.zcontent_len = z2zE623;
      z3zE944.zcount = z2zE625;
      zupdated = z3zE944;
    }
    {
      z8zE641 = zbal_storage_read_groups_sizze(zaccount, zread_count, zchange_count, znext_read, znext_change, zupdated);
      if (have_exception) {  goto end_block_exception_1424;  }
    }
  } else {  z8zE641 = zresult;  }
end_function_1423: ;
  return z8zE641;
end_block_exception_1424: ;
  struct zBalContentCount z8zE1104 = { .zcontent_len = UINT64_C(0xdeadc0de), .zcount = UINT64_C(0xdeadc0de) };
  return z8zE1104;
}

unit zbal_write_storage_read_groups(uint64_t zaccount, uint64_t zread_count, uint64_t zchange_count, uint64_t zread, uint64_t zchange)
{
  unit z8zE642;
  bool z2zE616;
  z2zE616 = (zread < zread_count);
  if (z2zE616) {
    sail_u256 zslot;
    zslot = bal_storage_read_slot(zaccount, zread);
    uint64_t znext_read;
    znext_read = zbal_storage_read_run_end(zaccount, zread_count, zslot, zread);
    uint64_t znext_change;
    znext_change = zbal_storage_change_seek(zaccount, zchange_count, zslot, zchange);
    bool zchanged;
    {
      bool z2zE618;
      z2zE618 = (znext_change < zchange_count);
      if (z2zE618) {
        sail_u256 z2zE619;
        z2zE619 = bal_storage_change_slot(zaccount, znext_change);
        zchanged = eq_u256(z2zE619, zslot);
      } else {  zchanged = false;  }
    }
    bool z2zE617;
    z2zE617 = not(zchanged);
    unit z3zE943;
    if (z2zE617) {  z3zE943 = zrlp_write_uint_word(zslot);  } else {  z3zE943 = UNIT;  }
    z8zE642 = zbal_write_storage_read_groups(zaccount, zread_count, zchange_count, znext_read, znext_change);
  } else {  z8zE642 = UNIT;  }
end_function_1421: ;
  return z8zE642;
end_block_exception_1422: ;

  return UNIT;
}

struct zBalContentCount zbal_storage_reads_sizze(uint64_t zaccount)
{
  struct zBalContentCount z8zE643;
  uint64_t z2zE614;
  z2zE614 = bal_storage_read_count(zaccount);
  uint64_t z2zE615;
  z2zE615 = bal_storage_change_count(zaccount);
  struct zBalContentCount z3zE942;
  z3zE942.zcontent_len = zBAL_RLP_ZERO;
  z3zE942.zcount = UINT64_C(0);
  {
    z8zE643 = zbal_storage_read_groups_sizze(zaccount, z2zE614, z2zE615, UINT64_C(0), UINT64_C(0), z3zE942);
    if (have_exception) {  goto end_block_exception_1420;  }
  }
end_function_1419: ;
  return z8zE643;
end_block_exception_1420: ;
  struct zBalContentCount z8zE1105 = { .zcontent_len = UINT64_C(0xdeadc0de), .zcount = UINT64_C(0xdeadc0de) };
  return z8zE1105;
}

unit zbal_write_storage_reads(uint64_t zaccount, struct zBalContentCount zsizze)
{
  unit z8zE644;
  uint64_t z2zE611;
  {
    uint64_t z2zE610;
    z2zE610 = zsizze.zcontent_len;
    z2zE611 = zbal_rlp_length_to_byte_length(z2zE610);
  }
  unit z3zE941;
  z3zE941 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE611);
  uint64_t z2zE612;
  z2zE612 = bal_storage_read_count(zaccount);
  uint64_t z2zE613;
  z2zE613 = bal_storage_change_count(zaccount);
  z8zE644 = zbal_write_storage_read_groups(zaccount, z2zE612, z2zE613, UINT64_C(0), UINT64_C(0));
end_function_1417: ;
  return z8zE644;
end_block_exception_1418: ;

  return UNIT;
}

uint64_t zbal_balance_run_end(uint64_t zaccount, uint64_t zcount, uint64_t zindex, uint64_t zcursor)
{
  uint64_t z8zE645;
  bool z2zE606;
  z2zE606 = (zcursor < zcount);
  if (z2zE606) {
    bool z2zE608;
    {
      uint64_t z2zE607;
      z2zE607 = bal_balance_change_index(zaccount, zcursor);
      z2zE608 = (z2zE607 == zindex);
    }
    if (z2zE608) {
      uint64_t z2zE609;
      {    z2zE609 = (zcursor + UINT64_C(1));
      }
      z8zE645 = zbal_balance_run_end(zaccount, zcount, zindex, z2zE609);
    } else {  z8zE645 = zcursor;  }
  } else {  z8zE645 = zcursor;  }
end_function_1415: ;
  return z8zE645;
end_block_exception_1416: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zbal_balance_groups_sizze(uint64_t zaccount, uint64_t zcount, uint64_t zcursor, uint64_t zcontent_len)
{
  uint64_t z8zE646;
  bool z2zE603;
  z2zE603 = (zcursor < zcount);
  if (z2zE603) {
    uint64_t zindex;
    zindex = bal_balance_change_index(zaccount, zcursor);
    uint64_t znext;
    znext = zbal_balance_run_end(zaccount, zcount, zindex, zcursor);
    uint64_t zlast;
    {
      zlast = zbal_previous_index(znext);
      if (have_exception) {  goto end_block_exception_1414;  }
    }
    uint64_t znext_length;
    {
      uint64_t z2zE605;
      {
        sail_u256 z2zE604;
        z2zE604 = bal_balance_change_value(zaccount, zlast);
        {
          z2zE605 = zbal_index_word_sizze(zindex, z2zE604);
          if (have_exception) {  goto end_block_exception_1414;  }
        }
      }
      {
        znext_length = zbal_rlp_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_len, z2zE605);
        if (have_exception) {  goto end_block_exception_1414;  }
      }
    }
    {
      z8zE646 = zbal_balance_groups_sizze(zaccount, zcount, znext, znext_length);
      if (have_exception) {  goto end_block_exception_1414;  }
    }
  } else {  z8zE646 = zcontent_len;  }
end_function_1413: ;
  return z8zE646;
end_block_exception_1414: ;

  return UINT64_C(0xdeadc0de);
}

unit zbal_write_balance_groups(uint64_t zaccount, uint64_t zcount, uint64_t zcursor)
{
  unit z8zE647;
  bool z2zE601;
  z2zE601 = (zcursor < zcount);
  if (z2zE601) {
    uint64_t zindex;
    zindex = bal_balance_change_index(zaccount, zcursor);
    uint64_t znext;
    znext = zbal_balance_run_end(zaccount, zcount, zindex, zcursor);
    uint64_t zlast;
    {
      zlast = zbal_previous_index(znext);
      if (have_exception) {  goto end_block_exception_1412;  }
    }
    sail_u256 z2zE602;
    z2zE602 = bal_balance_change_value(zaccount, zlast);
    unit z3zE940;
    {
      z3zE940 = zbal_write_index_word(zindex, z2zE602);
      if (have_exception) {  goto end_block_exception_1412;  }
    }
    {
      z8zE647 = zbal_write_balance_groups(zaccount, zcount, znext);
      if (have_exception) {  goto end_block_exception_1412;  }
    }
  } else {  z8zE647 = UNIT;  }
end_function_1411: ;
  return z8zE647;
end_block_exception_1412: ;

  return UNIT;
}

uint64_t zbal_balance_changes_sizze(uint64_t zaccount)
{
  uint64_t z8zE648;
  uint64_t z2zE600;
  z2zE600 = bal_balance_change_count(zaccount);
  {
    z8zE648 = zbal_balance_groups_sizze(zaccount, z2zE600, UINT64_C(0), zBAL_RLP_ZERO);
    if (have_exception) {  goto end_block_exception_1410;  }
  }
end_function_1409: ;
  return z8zE648;
end_block_exception_1410: ;

  return UINT64_C(0xdeadc0de);
}

unit zbal_write_balance_changes(uint64_t zaccount, uint64_t zcontent_len)
{
  unit z8zE649;
  uint64_t z2zE598;
  z2zE598 = zbal_rlp_length_to_byte_length(zcontent_len);
  unit z3zE939;
  z3zE939 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE598);
  uint64_t z2zE599;
  z2zE599 = bal_balance_change_count(zaccount);
  {
    z8zE649 = zbal_write_balance_groups(zaccount, z2zE599, UINT64_C(0));
    if (have_exception) {  goto end_block_exception_1408;  }
  }
end_function_1407: ;
  return z8zE649;
end_block_exception_1408: ;

  return UNIT;
}

struct zBalNonceRun zbal_nonce_run(uint64_t zaccount, uint64_t zcount, uint64_t zindex, uint64_t zcursor, uint64_t zmaximum)
{
  struct zBalNonceRun z8zE650;
  bool z2zE593;
  z2zE593 = (zcursor < zcount);
  if (z2zE593) {
    bool z2zE595;
    {
      uint64_t z2zE594;
      z2zE594 = bal_nonce_change_index(zaccount, zcursor);
      z2zE595 = (z2zE594 == zindex);
    }
    if (z2zE595) {
      uint64_t zvalue;
      zvalue = bal_nonce_change_value(zaccount, zcursor);
      uint64_t znext_maximum;
      {
        bool z2zE597;
        z2zE597 = (zmaximum < zvalue);
        if (z2zE597) {  znext_maximum = zvalue;  } else {  znext_maximum = zmaximum;  }
      }
      uint64_t z2zE596;
      {    z2zE596 = (zcursor + UINT64_C(1));
      }
      z8zE650 = zbal_nonce_run(zaccount, zcount, zindex, z2zE596, znext_maximum);
    } else {
      struct zBalNonceRun z3zE938;
      z3zE938.zcursor = zcursor;
      z3zE938.zmaximum = zmaximum;
      z8zE650 = z3zE938;
    }
  } else {
    struct zBalNonceRun z3zE937;
    z3zE937.zcursor = zcursor;
    z3zE937.zmaximum = zmaximum;
    z8zE650 = z3zE937;
  }
end_function_1405: ;
  return z8zE650;
end_block_exception_1406: ;
  struct zBalNonceRun z8zE1106 = { .zcursor = UINT64_C(0xdeadc0de), .zmaximum = UINT64_C(0xdeadc0de) };
  return z8zE1106;
}

uint64_t zbal_nonce_groups_sizze(uint64_t zaccount, uint64_t zcount, uint64_t zcursor, uint64_t zcontent_len)
{
  uint64_t z8zE651;
  bool z2zE588;
  z2zE588 = (zcursor < zcount);
  if (z2zE588) {
    uint64_t zindex;
    zindex = bal_nonce_change_index(zaccount, zcursor);
    uint64_t zfirst;
    zfirst = bal_nonce_change_value(zaccount, zcursor);
    struct zBalNonceRun zrun;
    {
      uint64_t z2zE592;
      {    z2zE592 = (zcursor + UINT64_C(1));
      }
      zrun = zbal_nonce_run(zaccount, zcount, zindex, z2zE592, zfirst);
    }
    uint64_t znext_length;
    {
      uint64_t z2zE591;
      {
        uint64_t z2zE590;
        z2zE590 = zrun.zmaximum;
        {
          z2zE591 = zbal_index_nonce_sizze(zindex, z2zE590);
          if (have_exception) {  goto end_block_exception_1404;  }
        }
      }
      {
        znext_length = zbal_rlp_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_len, z2zE591);
        if (have_exception) {  goto end_block_exception_1404;  }
      }
    }
    uint64_t z2zE589;
    z2zE589 = zrun.zcursor;
    {
      z8zE651 = zbal_nonce_groups_sizze(zaccount, zcount, z2zE589, znext_length);
      if (have_exception) {  goto end_block_exception_1404;  }
    }
  } else {  z8zE651 = zcontent_len;  }
end_function_1403: ;
  return z8zE651;
end_block_exception_1404: ;

  return UINT64_C(0xdeadc0de);
}

unit zbal_write_nonce_groups(uint64_t zaccount, uint64_t zcount, uint64_t zcursor)
{
  unit z8zE652;
  bool z2zE584;
  z2zE584 = (zcursor < zcount);
  if (z2zE584) {
    uint64_t zindex;
    zindex = bal_nonce_change_index(zaccount, zcursor);
    uint64_t zfirst;
    zfirst = bal_nonce_change_value(zaccount, zcursor);
    struct zBalNonceRun zrun;
    {
      uint64_t z2zE587;
      {    z2zE587 = (zcursor + UINT64_C(1));
      }
      zrun = zbal_nonce_run(zaccount, zcount, zindex, z2zE587, zfirst);
    }
    uint64_t z2zE585;
    z2zE585 = zrun.zmaximum;
    unit z3zE936;
    {
      z3zE936 = zbal_write_index_nonce(zindex, z2zE585);
      if (have_exception) {  goto end_block_exception_1402;  }
    }
    uint64_t z2zE586;
    z2zE586 = zrun.zcursor;
    {
      z8zE652 = zbal_write_nonce_groups(zaccount, zcount, z2zE586);
      if (have_exception) {  goto end_block_exception_1402;  }
    }
  } else {  z8zE652 = UNIT;  }
end_function_1401: ;
  return z8zE652;
end_block_exception_1402: ;

  return UNIT;
}

uint64_t zbal_nonce_changes_sizze(uint64_t zaccount)
{
  uint64_t z8zE653;
  uint64_t z2zE583;
  z2zE583 = bal_nonce_change_count(zaccount);
  {
    z8zE653 = zbal_nonce_groups_sizze(zaccount, z2zE583, UINT64_C(0), zBAL_RLP_ZERO);
    if (have_exception) {  goto end_block_exception_1400;  }
  }
end_function_1399: ;
  return z8zE653;
end_block_exception_1400: ;

  return UINT64_C(0xdeadc0de);
}

unit zbal_write_nonce_changes(uint64_t zaccount, uint64_t zcontent_len)
{
  unit z8zE654;
  uint64_t z2zE581;
  z2zE581 = zbal_rlp_length_to_byte_length(zcontent_len);
  unit z3zE935;
  z3zE935 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE581);
  uint64_t z2zE582;
  z2zE582 = bal_nonce_change_count(zaccount);
  {
    z8zE654 = zbal_write_nonce_groups(zaccount, z2zE582, UINT64_C(0));
    if (have_exception) {  goto end_block_exception_1398;  }
  }
end_function_1397: ;
  return z8zE654;
end_block_exception_1398: ;

  return UNIT;
}

uint64_t zbal_code_run_end(uint64_t zaccount, uint64_t zcount, uint64_t zindex, uint64_t zcursor)
{
  uint64_t z8zE655;
  bool z2zE577;
  z2zE577 = (zcursor < zcount);
  if (z2zE577) {
    bool z2zE579;
    {
      uint64_t z2zE578;
      z2zE578 = bal_code_change_index(zaccount, zcursor);
      z2zE579 = (z2zE578 == zindex);
    }
    if (z2zE579) {
      uint64_t z2zE580;
      {    z2zE580 = (zcursor + UINT64_C(1));
      }
      z8zE655 = zbal_code_run_end(zaccount, zcount, zindex, z2zE580);
    } else {  z8zE655 = zcursor;  }
  } else {  z8zE655 = zcursor;  }
end_function_1395: ;
  return z8zE655;
end_block_exception_1396: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zbal_code_groups_sizze(uint64_t zaccount, uint64_t zcount, uint64_t zcursor, uint64_t zcontent_len)
{
  uint64_t z8zE656;
  bool z2zE574;
  z2zE574 = (zcursor < zcount);
  if (z2zE574) {
    uint64_t zindex;
    zindex = bal_code_change_index(zaccount, zcursor);
    uint64_t znext;
    znext = zbal_code_run_end(zaccount, zcount, zindex, zcursor);
    uint64_t zlast;
    {
      zlast = zbal_previous_index(znext);
      if (have_exception) {  goto end_block_exception_1394;  }
    }
    uint64_t znext_length;
    {
      uint64_t z2zE576;
      {
        sail_fixed_bytes_32 z2zE575;
        z2zE575 = bal_code_change_hash(zaccount, zlast);
        {
          z2zE576 = zbal_index_code_sizze(zindex, z2zE575);
          if (have_exception) {  goto end_block_exception_1394;  }
        }
      }
      {
        znext_length = zbal_rlp_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_len, z2zE576);
        if (have_exception) {  goto end_block_exception_1394;  }
      }
    }
    {
      z8zE656 = zbal_code_groups_sizze(zaccount, zcount, znext, znext_length);
      if (have_exception) {  goto end_block_exception_1394;  }
    }
  } else {  z8zE656 = zcontent_len;  }
end_function_1393: ;
  return z8zE656;
end_block_exception_1394: ;

  return UINT64_C(0xdeadc0de);
}

unit zbal_write_code_groups(uint64_t zaccount, uint64_t zcount, uint64_t zcursor)
{
  unit z8zE657;
  bool z2zE572;
  z2zE572 = (zcursor < zcount);
  if (z2zE572) {
    uint64_t zindex;
    zindex = bal_code_change_index(zaccount, zcursor);
    uint64_t znext;
    znext = zbal_code_run_end(zaccount, zcount, zindex, zcursor);
    uint64_t zlast;
    {
      zlast = zbal_previous_index(znext);
      if (have_exception) {  goto end_block_exception_1392;  }
    }
    sail_fixed_bytes_32 z2zE573;
    z2zE573 = bal_code_change_hash(zaccount, zlast);
    unit z3zE934;
    {
      z3zE934 = zbal_write_index_code(zindex, z2zE573);
      if (have_exception) {  goto end_block_exception_1392;  }
    }
    {
      z8zE657 = zbal_write_code_groups(zaccount, zcount, znext);
      if (have_exception) {  goto end_block_exception_1392;  }
    }
  } else {  z8zE657 = UNIT;  }
end_function_1391: ;
  return z8zE657;
end_block_exception_1392: ;

  return UNIT;
}

uint64_t zbal_code_changes_sizze(uint64_t zaccount)
{
  uint64_t z8zE658;
  uint64_t z2zE571;
  z2zE571 = bal_code_change_count(zaccount);
  {
    z8zE658 = zbal_code_groups_sizze(zaccount, z2zE571, UINT64_C(0), zBAL_RLP_ZERO);
    if (have_exception) {  goto end_block_exception_1390;  }
  }
end_function_1389: ;
  return z8zE658;
end_block_exception_1390: ;

  return UINT64_C(0xdeadc0de);
}

unit zbal_write_code_changes(uint64_t zaccount, uint64_t zcontent_len)
{
  unit z8zE659;
  uint64_t z2zE569;
  z2zE569 = zbal_rlp_length_to_byte_length(zcontent_len);
  unit z3zE933;
  z3zE933 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE569);
  uint64_t z2zE570;
  z2zE570 = bal_code_change_count(zaccount);
  {
    z8zE659 = zbal_write_code_groups(zaccount, z2zE570, UINT64_C(0));
    if (have_exception) {  goto end_block_exception_1388;  }
  }
end_function_1387: ;
  return z8zE659;
end_block_exception_1388: ;

  return UNIT;
}

struct zBalAccountSizze zbal_account_sizze(uint64_t zaccount)
{
  struct zBalAccountSizze z8zE660;
  struct zBalContentCount zstorage_changes;
  {
    zstorage_changes = zbal_storage_changes_sizze(zaccount);
    if (have_exception) {  goto end_block_exception_1386;  }
  }
  struct zBalContentCount zstorage_reads;
  {
    zstorage_reads = zbal_storage_reads_sizze(zaccount);
    if (have_exception) {  goto end_block_exception_1386;  }
  }
  uint64_t zbalance_changes_len;
  {
    zbalance_changes_len = zbal_balance_changes_sizze(zaccount);
    if (have_exception) {  goto end_block_exception_1386;  }
  }
  uint64_t znonce_changes_len;
  {
    znonce_changes_len = zbal_nonce_changes_sizze(zaccount);
    if (have_exception) {  goto end_block_exception_1386;  }
  }
  uint64_t zcode_changes_len;
  {
    zcode_changes_len = zbal_code_changes_sizze(zaccount);
    if (have_exception) {  goto end_block_exception_1386;  }
  }
  uint64_t zcontent_len;
  zcontent_len = UINT64_C(21);
  uint64_t z2zE554;
  {
    uint64_t z2zE553;
    {
      uint64_t z2zE552;
      z2zE552 = zstorage_changes.zcontent_len;
      {
        z2zE553 = zbal_rlp_list_sizze(z2zE552);
        if (have_exception) {  goto end_block_exception_1386;  }
      }
    }
    z2zE554 = zbal_rlp_length_to_byte_length(z2zE553);
  }
  {
    zcontent_len = zbal_rlp_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_len, z2zE554);
    if (have_exception) {  goto end_block_exception_1386;  }
  }
  unit z3zE931;
  z3zE931 = UNIT;
  uint64_t z2zE557;
  {
    uint64_t z2zE556;
    {
      uint64_t z2zE555;
      z2zE555 = zstorage_reads.zcontent_len;
      {
        z2zE556 = zbal_rlp_list_sizze(z2zE555);
        if (have_exception) {  goto end_block_exception_1386;  }
      }
    }
    z2zE557 = zbal_rlp_length_to_byte_length(z2zE556);
  }
  {
    zcontent_len = zbal_rlp_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_len, z2zE557);
    if (have_exception) {  goto end_block_exception_1386;  }
  }
  unit z3zE930;
  z3zE930 = UNIT;
  uint64_t z2zE559;
  {
    uint64_t z2zE558;
    {
      z2zE558 = zbal_rlp_list_sizze(zbalance_changes_len);
      if (have_exception) {  goto end_block_exception_1386;  }
    }
    z2zE559 = zbal_rlp_length_to_byte_length(z2zE558);
  }
  {
    zcontent_len = zbal_rlp_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_len, z2zE559);
    if (have_exception) {  goto end_block_exception_1386;  }
  }
  unit z3zE929;
  z3zE929 = UNIT;
  uint64_t z2zE561;
  {
    uint64_t z2zE560;
    {
      z2zE560 = zbal_rlp_list_sizze(znonce_changes_len);
      if (have_exception) {  goto end_block_exception_1386;  }
    }
    z2zE561 = zbal_rlp_length_to_byte_length(z2zE560);
  }
  {
    zcontent_len = zbal_rlp_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_len, z2zE561);
    if (have_exception) {  goto end_block_exception_1386;  }
  }
  unit z3zE928;
  z3zE928 = UNIT;
  uint64_t z2zE563;
  {
    uint64_t z2zE562;
    {
      z2zE562 = zbal_rlp_list_sizze(zcode_changes_len);
      if (have_exception) {  goto end_block_exception_1386;  }
    }
    z2zE563 = zbal_rlp_length_to_byte_length(z2zE562);
  }
  {
    zcontent_len = zbal_rlp_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_len, z2zE563);
    if (have_exception) {  goto end_block_exception_1386;  }
  }
  unit z3zE927;
  z3zE927 = UNIT;
  uint64_t z2zE564;
  {
    z2zE564 = zbal_rlp_list_sizze(zcontent_len);
    if (have_exception) {  goto end_block_exception_1386;  }
  }
  uint64_t z2zE568;
  {
    uint64_t z2zE566;
    {
      uint64_t z2zE565;
      z2zE565 = zstorage_changes.zcount;
      {
        z2zE566 = zbal_count_addzIreprzGU64zCU64zCU64zKzIboundszG43fd78b15b770f2258241a12de1a7119zK(UINT64_C(1), z2zE565);
        if (have_exception) {  goto end_block_exception_1386;  }
      }
    }
    uint64_t z2zE567;
    z2zE567 = zstorage_reads.zcount;
    {
      z2zE568 = zbal_count_addzIreprzGU64zCU64zCU64zKzIboundszGd0328719294598864996af0723f2d8e9zK(z2zE566, z2zE567);
      if (have_exception) {  goto end_block_exception_1386;  }
    }
  }
  struct zBalAccountSizze z3zE932;
  z3zE932.zencoded_len = z2zE564;
  z3zE932.zitem_count = z2zE568;
  z8zE660 = z3zE932;
end_function_1385: ;
  return z8zE660;
end_block_exception_1386: ;
  struct zBalAccountSizze z8zE1107 = { .zencoded_len = UINT64_C(0xdeadc0de), .zitem_count = UINT64_C(0xdeadc0de) };
  return z8zE1107;
}

unit zbal_write_account(uint64_t zaccount)
{
  unit z8zE661;
  struct zBalContentCount zstorage_changes;
  {
    zstorage_changes = zbal_storage_changes_sizze(zaccount);
    if (have_exception) {  goto end_block_exception_1384;  }
  }
  struct zBalContentCount zstorage_reads;
  {
    zstorage_reads = zbal_storage_reads_sizze(zaccount);
    if (have_exception) {  goto end_block_exception_1384;  }
  }
  uint64_t zbalance_changes_len;
  {
    zbalance_changes_len = zbal_balance_changes_sizze(zaccount);
    if (have_exception) {  goto end_block_exception_1384;  }
  }
  uint64_t znonce_changes_len;
  {
    znonce_changes_len = zbal_nonce_changes_sizze(zaccount);
    if (have_exception) {  goto end_block_exception_1384;  }
  }
  uint64_t zcode_changes_len;
  {
    zcode_changes_len = zbal_code_changes_sizze(zaccount);
    if (have_exception) {  goto end_block_exception_1384;  }
  }
  uint64_t zcontent_len;
  zcontent_len = UINT64_C(21);
  uint64_t z2zE540;
  {
    uint64_t z2zE539;
    {
      uint64_t z2zE538;
      z2zE538 = zstorage_changes.zcontent_len;
      {
        z2zE539 = zbal_rlp_list_sizze(z2zE538);
        if (have_exception) {  goto end_block_exception_1384;  }
      }
    }
    z2zE540 = zbal_rlp_length_to_byte_length(z2zE539);
  }
  {
    zcontent_len = zbal_rlp_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_len, z2zE540);
    if (have_exception) {  goto end_block_exception_1384;  }
  }
  unit z3zE926;
  z3zE926 = UNIT;
  uint64_t z2zE543;
  {
    uint64_t z2zE542;
    {
      uint64_t z2zE541;
      z2zE541 = zstorage_reads.zcontent_len;
      {
        z2zE542 = zbal_rlp_list_sizze(z2zE541);
        if (have_exception) {  goto end_block_exception_1384;  }
      }
    }
    z2zE543 = zbal_rlp_length_to_byte_length(z2zE542);
  }
  {
    zcontent_len = zbal_rlp_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_len, z2zE543);
    if (have_exception) {  goto end_block_exception_1384;  }
  }
  unit z3zE925;
  z3zE925 = UNIT;
  uint64_t z2zE545;
  {
    uint64_t z2zE544;
    {
      z2zE544 = zbal_rlp_list_sizze(zbalance_changes_len);
      if (have_exception) {  goto end_block_exception_1384;  }
    }
    z2zE545 = zbal_rlp_length_to_byte_length(z2zE544);
  }
  {
    zcontent_len = zbal_rlp_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_len, z2zE545);
    if (have_exception) {  goto end_block_exception_1384;  }
  }
  unit z3zE924;
  z3zE924 = UNIT;
  uint64_t z2zE547;
  {
    uint64_t z2zE546;
    {
      z2zE546 = zbal_rlp_list_sizze(znonce_changes_len);
      if (have_exception) {  goto end_block_exception_1384;  }
    }
    z2zE547 = zbal_rlp_length_to_byte_length(z2zE546);
  }
  {
    zcontent_len = zbal_rlp_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_len, z2zE547);
    if (have_exception) {  goto end_block_exception_1384;  }
  }
  unit z3zE923;
  z3zE923 = UNIT;
  uint64_t z2zE549;
  {
    uint64_t z2zE548;
    {
      z2zE548 = zbal_rlp_list_sizze(zcode_changes_len);
      if (have_exception) {  goto end_block_exception_1384;  }
    }
    z2zE549 = zbal_rlp_length_to_byte_length(z2zE548);
  }
  {
    zcontent_len = zbal_rlp_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zcontent_len, z2zE549);
    if (have_exception) {  goto end_block_exception_1384;  }
  }
  unit z3zE922;
  z3zE922 = UNIT;
  uint64_t z2zE550;
  z2zE550 = zbal_rlp_length_to_byte_length(zcontent_len);
  unit z3zE921;
  z3zE921 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE550);
  sail_fixed_bytes_20 z2zE551;
  z2zE551 = bal_account_address(zaccount);
  unit z3zE920;
  z3zE920 = zrlp_write_addr(z2zE551);
  unit z3zE919;
  {
    z3zE919 = zbal_write_storage_changes(zaccount, zstorage_changes);
    if (have_exception) {  goto end_block_exception_1384;  }
  }
  unit z3zE918;
  z3zE918 = zbal_write_storage_reads(zaccount, zstorage_reads);
  unit z3zE917;
  {
    z3zE917 = zbal_write_balance_changes(zaccount, zbalance_changes_len);
    if (have_exception) {  goto end_block_exception_1384;  }
  }
  unit z3zE916;
  {
    z3zE916 = zbal_write_nonce_changes(zaccount, znonce_changes_len);
    if (have_exception) {  goto end_block_exception_1384;  }
  }
  {
    z8zE661 = zbal_write_code_changes(zaccount, zcode_changes_len);
    if (have_exception) {  goto end_block_exception_1384;  }
  }
end_function_1383: ;
  return z8zE661;
end_block_exception_1384: ;

  return UNIT;
}

struct zEncodedBlockAccessList zencode_block_access_list(unit z3zE908)
{
  struct zEncodedBlockAccessList z8zE662;
  unit z3zE909;
  z3zE909 = bal_prepare(UNIT);
  uint64_t zaccount_count;
  zaccount_count = bal_account_count(UNIT);
  struct zBalContentCount zsizze;
  {
    struct zBalContentCount z3zE910;
    z3zE910.zcontent_len = zBAL_RLP_ZERO;
    z3zE910.zcount = UINT64_C(0);
    {
      zsizze = zbal_accounts_sizzezIreprzGU64zCU64zCRBalContentCountzCRBalContentCountzKzIboundszGa3760b966d2df2175e3b9bd94efd7f00zK(zaccount_count, UINT64_C(0), z3zE910);
      if (have_exception) {  goto end_block_exception_1378;  }
    }
  }
  uint64_t zstart;
  zstart = zscratch_begin(UNIT);
  uint64_t z2zE524;
  {
    uint64_t z2zE523;
    z2zE523 = zsizze.zcontent_len;
    z2zE524 = zbal_rlp_length_to_byte_length(z2zE523);
  }
  unit z3zE912;
  z3zE912 = zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE524);
  unit z3zE911;
  {
    z3zE911 = zbal_write_accountszIreprzGU64zCU64zCuzKzIboundszG6940b3315867b733b4ae91c7ea9466eczK(zaccount_count, UINT64_C(0));
    if (have_exception) {  goto end_block_exception_1378;  }
  }
  struct zByteSliceFields z2zE525;
  z2zE525 = zrlp_finish(zstart);
  uint64_t z2zE526;
  z2zE526 = zsizze.zcount;
  struct zEncodedBlockAccessList z3zE913;
  z3zE913.zbytes = z2zE525;
  z3zE913.zitem_count = z2zE526;
  z8zE662 = z3zE913;
end_function_1377: ;
  return z8zE662;
end_block_exception_1378: ;
  struct zByteSliceFields z8zE1109 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  struct zEncodedBlockAccessList z8zE1108 = { .zbytes = z8zE1109, .zitem_count = UINT64_C(0xdeadc0de) };
  return z8zE1108;
}

