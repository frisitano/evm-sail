/* Generated from sail/evm/transaction.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void create_letbind_218(void) {

  uint64_t z3zE239;
  z3zE239 = UINT64_C(21000);
  zG_transaction = z3zE239;
let_end_731: ;
}
void kill_letbind_218(void) {
}

void create_letbind_219(void) {

  uint64_t z3zE240;
  z3zE240 = UINT64_C(32000);
  zG_txcreate = z3zE240;
let_end_732: ;
}
void kill_letbind_219(void) {
}

void create_letbind_220(void) {

  uint64_t z3zE241;
  z3zE241 = UINT64_C(4);
  zG_txdatazzero = z3zE241;
let_end_733: ;
}
void kill_letbind_220(void) {
}

void create_letbind_221(void) {

  uint64_t z3zE242;
  z3zE242 = UINT64_C(16);
  zG_txdatanonzzero = z3zE242;
let_end_734: ;
}
void kill_letbind_221(void) {
}

void create_letbind_222(void) {

  uint64_t z3zE243;
  z3zE243 = UINT64_C(2400);
  zG_access_list_address = z3zE243;
let_end_735: ;
}
void kill_letbind_222(void) {
}

void create_letbind_223(void) {

  uint64_t z3zE244;
  z3zE244 = UINT64_C(1900);
  zG_access_list_storage_key = z3zE244;
let_end_736: ;
}
void kill_letbind_223(void) {
}

void create_letbind_224(void) {

  uint64_t z3zE245;
  z3zE245 = UINT64_C(12500);
  zPER_AUTH_BASE = z3zE245;
let_end_737: ;
}
void kill_letbind_224(void) {
}

void create_letbind_225(void) {

  uint64_t z3zE246;
  z3zE246 = UINT64_C(25000);
  zPER_EMPTY_ACCOUNT = z3zE246;
let_end_738: ;
}
void kill_letbind_225(void) {
}

void create_letbind_226(void) {

  uint64_t z3zE247;
  z3zE247 = UINT64_C(12000);
  zAMSTERDAM_TX_BASE = z3zE247;
let_end_739: ;
}
void kill_letbind_226(void) {
}

void create_letbind_227(void) {

  uint64_t z3zE248;
  z3zE248 = UINT64_C(11000);
  zAMSTERDAM_CREATE_ACCESS = z3zE248;
let_end_740: ;
}
void kill_letbind_227(void) {
}

void create_letbind_228(void) {

  uint64_t z3zE249;
  z3zE249 = UINT64_C(3000);
  zAMSTERDAM_COLD_ACCOUNT_ACCESS = z3zE249;
let_end_741: ;
}
void kill_letbind_228(void) {
}

void create_letbind_229(void) {

  uint64_t z3zE250;
  z3zE250 = UINT64_C(4244);
  zAMSTERDAM_TX_VALUE_COST = z3zE250;
let_end_742: ;
}
void kill_letbind_229(void) {
}

void create_letbind_230(void) {

  uint64_t z3zE251;
  z3zE251 = UINT64_C(1756);
  zAMSTERDAM_TRANSFER_LOG_COST = z3zE251;
let_end_743: ;
}
void kill_letbind_230(void) {
}

void create_letbind_231(void) {

  uint64_t z3zE252;
  z3zE252 = UINT64_C(3000);
  zAMSTERDAM_ACCESS_LIST_ADDRESS = z3zE252;
let_end_744: ;
}
void kill_letbind_231(void) {
}

void create_letbind_232(void) {

  uint64_t z3zE253;
  z3zE253 = UINT64_C(3000);
  zAMSTERDAM_ACCESS_LIST_SLOT = z3zE253;
let_end_745: ;
}
void kill_letbind_232(void) {
}

void create_letbind_233(void) {

  uint64_t z3zE254;
  z3zE254 = UINT64_C(1280);
  zAMSTERDAM_ACCESS_LIST_ADDRESS_FLOOR = z3zE254;
let_end_746: ;
}
void kill_letbind_233(void) {
}

void create_letbind_234(void) {

  uint64_t z3zE255;
  z3zE255 = UINT64_C(2048);
  zAMSTERDAM_ACCESS_LIST_SLOT_FLOOR = z3zE255;
let_end_747: ;
}
void kill_letbind_234(void) {
}

void create_letbind_235(void) {

  uint64_t z3zE256;
  z3zE256 = UINT64_C(7816);
  zAMSTERDAM_AUTH_BASE = z3zE256;
let_end_748: ;
}
void kill_letbind_235(void) {
}

void create_letbind_236(void) {

  uint64_t z3zE257;
  z3zE257 = UINT64_C(64);
  zAMSTERDAM_CALLDATA_FLOOR_BYTE = z3zE257;
let_end_749: ;
}
void kill_letbind_236(void) {
}

void create_letbind_237(void) {

  uint64_t z3zE258;
  z3zE258 = zTRANSACTION_EXECUTION_GAS_LIMIT;
  zAMSTERDAM_TX_MAX_GAS = z3zE258;
let_end_750: ;
}
void kill_letbind_237(void) {
}

struct zByteSliceFields ztransaction_initcode_slice(struct zByteSliceFields zinput)
{
  struct zByteSliceFields z8zE489;
  z8zE489 = zcode_slice(zinput);
end_function_1942: ;
  return z8zE489;
end_block_exception_1943: ;
  struct zByteSliceFields z8zE885 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE885;
}

void create_letbind_238(void) {    CREATE(zAmsterdamAuthorizzationState)(&zEMPTY_AMSTERDAM_AUTHORIZATION_STATE);

  struct zAmsterdamAuthorizzationState z3zE263;
  CREATE(zAmsterdamAuthorizzationState)(&z3zE263);
  struct zAmsterdamAuthorizzationState z3zE259;
  CREATE(zAmsterdamAuthorizzationState)(&z3zE259);
  zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z3zE260;
  CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE260);
  COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE259)->zdelegation_set_for), z3zE260);
  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE260);
  zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z3zE261;
  CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE261);
  COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE259)->zoriginally_delegated), z3zE261);
  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE261);
  zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z3zE262;
  CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE262);
  COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE259)->zseen_valid_authorities), z3zE262);
  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE262);
  COPY(zAmsterdamAuthorizzationState)(&z3zE263, z3zE259);
  KILL(zAmsterdamAuthorizzationState)(&z3zE259);
  COPY(zAmsterdamAuthorizzationState)(&zEMPTY_AMSTERDAM_AUTHORIZATION_STATE, z3zE263);
  KILL(zAmsterdamAuthorizzationState)(&z3zE263);
let_end_752: ;
}
void kill_letbind_238(void) {    KILL(zAmsterdamAuthorizzationState)(&zEMPTY_AMSTERDAM_AUTHORIZATION_STATE);
}

uint64_t zcalldata_cost(struct zByteSliceFields zinput)
{
  uint64_t z8zE490;
  uint64_t znonzzeroes;
  znonzzeroes = zslice_count_nonzzero(zinput);
  uint64_t zinput_len;
  zinput_len = zinput.zlen;
  bool z2zE1627;
  z2zE1627 = (!(zinput_len < znonzzeroes));
  if (z2zE1627) {
    uint64_t zzzeroes;
    {    zzzeroes = (zinput_len - znonzzeroes);
    }
    uint64_t z2zE1628;
    {    z2zE1628 = (zG_txdatazzero * zzzeroes);
    }
    uint64_t z2zE1629;
    {    z2zE1629 = (zG_txdatanonzzero * znonzzeroes);
    }
    {    z8zE490 = (z2zE1628 + z2zE1629);
    }
  } else {
    struct zexception z2zE1630;
    CREATE(zexception)(&z2zE1630);
    zInvalidBlock(&z2zE1630, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE1630);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/transaction.sail:105.8-105.44");
    KILL(zexception)(&z2zE1630);
    goto end_block_exception_1941;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1630);
  }
end_function_1940: ;
  return z8zE490;
end_block_exception_1941: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zvalidate_blob_hash_version_at(struct zBlobHashes zhashes, uint64_t zremaining, uint64_t zoffset)
{
  uint64_t z8zE491;
  bool z2zE1622;
  z2zE1622 = (zremaining == UINT64_C(0));
  if (z2zE1622) {  z8zE491 = UINT64_C(0);  } else {
    bool z2zE1625;
    {
      uint64_t z2zE1624;
      {
        struct zByteSliceFields z2zE1623;
        z2zE1623 = zhashes.zbytes;
        z2zE1624 = zslice_byte(z2zE1623, zoffset);
      }
      z2zE1625 = (z2zE1624 != UINT64_C(0x01));
    }
    unit z3zE1528;
    if (z2zE1625) {
      struct zexception z2zE1626;
      CREATE(zexception)(&z2zE1626);
      zInvalidBlock(&z2zE1626, zExecutionInvalid);
      COPY(zexception)(current_exception, z2zE1626);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/evm/transaction.sail:135.12-135.48");
      KILL(zexception)(&z2zE1626);
      goto end_block_exception_1939;
      /* unreachable after throw */
      KILL(zexception)(&z2zE1626);
    } else {  z3zE1528 = UNIT;  }
    z8zE491 = ztransaction_blob_count_decrement(zremaining);
  }
end_function_1938: ;
  return z8zE491;
end_block_exception_1939: ;

  return UINT64_C(0xdeadc0de);
}

unit zvalidate_blob_hash_versions(struct zBlobHashes zhashes)
{
  unit z8zE492;
  uint64_t zremaining;
  zremaining = zhashes.zcount;
  {
    zremaining = zvalidate_blob_hash_version_at(zhashes, zremaining, UINT64_C(1));
    if (have_exception) {  goto end_block_exception_1937;  }
  }
  unit z3zE1527;
  z3zE1527 = UNIT;
  {
    zremaining = zvalidate_blob_hash_version_at(zhashes, zremaining, UINT64_C(34));
    if (have_exception) {  goto end_block_exception_1937;  }
  }
  unit z3zE1526;
  z3zE1526 = UNIT;
  {
    zremaining = zvalidate_blob_hash_version_at(zhashes, zremaining, UINT64_C(67));
    if (have_exception) {  goto end_block_exception_1937;  }
  }
  unit z3zE1525;
  z3zE1525 = UNIT;
  {
    zremaining = zvalidate_blob_hash_version_at(zhashes, zremaining, UINT64_C(100));
    if (have_exception) {  goto end_block_exception_1937;  }
  }
  unit z3zE1524;
  z3zE1524 = UNIT;
  {
    zremaining = zvalidate_blob_hash_version_at(zhashes, zremaining, UINT64_C(133));
    if (have_exception) {  goto end_block_exception_1937;  }
  }
  unit z3zE1523;
  z3zE1523 = UNIT;
  {
    zremaining = zvalidate_blob_hash_version_at(zhashes, zremaining, UINT64_C(166));
    if (have_exception) {  goto end_block_exception_1937;  }
  }
  unit z3zE1522;
  z3zE1522 = UNIT;
  {
    zremaining = zvalidate_blob_hash_version_at(zhashes, zremaining, UINT64_C(199));
    if (have_exception) {  goto end_block_exception_1937;  }
  }
  unit z3zE1521;
  z3zE1521 = UNIT;
  {
    zremaining = zvalidate_blob_hash_version_at(zhashes, zremaining, UINT64_C(232));
    if (have_exception) {  goto end_block_exception_1937;  }
  }
  unit z3zE1520;
  z3zE1520 = UNIT;
  {
    zremaining = zvalidate_blob_hash_version_at(zhashes, zremaining, UINT64_C(265));
    if (have_exception) {  goto end_block_exception_1937;  }
  }
  unit z3zE1519;
  z3zE1519 = UNIT;
  bool z2zE1620;
  z2zE1620 = (zremaining != UINT64_C(0));
  if (z2zE1620) {
    struct zexception z2zE1621;
    CREATE(zexception)(&z2zE1621);
    zInvalidBlock(&z2zE1621, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE1621);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/transaction.sail:156.8-156.44");
    KILL(zexception)(&z2zE1621);
    goto end_block_exception_1937;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1621);
  } else {  z8zE492 = UNIT;  }
end_function_1936: ;
  return z8zE492;
end_block_exception_1937: ;

  return UNIT;
}

uint64_t zlegacy_intrinsic_gas(struct zTransaction ztx)
{
  uint64_t z8zE493;
  uint64_t zdata_cost;
  {
    struct zByteSliceFields z2zE1619;
    z2zE1619 = ztx.zinput_src;
    {
      zdata_cost = zcalldata_cost(z2zE1619);
      if (have_exception) {  goto end_block_exception_1935;  }
    }
  }
  struct zByteSliceFields zinput;
  zinput = ztx.zinput_src;
  uint64_t zinput_len;
  zinput_len = zinput.zlen;
  uint64_t zaddress_cost;
  {
    uint64_t z2zE1618;
    z2zE1618 = ztx.zaccess_list_address_count;
    {    zaddress_cost = (zG_access_list_address * z2zE1618);
    }
  }
  uint64_t zslot_cost;
  {
    uint64_t z2zE1617;
    z2zE1617 = ztx.zaccess_list_slot_count;
    {    zslot_cost = (zG_access_list_storage_key * z2zE1617);
    }
  }
  uint64_t zauth_cost;
  {
    uint64_t z2zE1616;
    z2zE1616 = ztx.zauthorizzation_count;
    {    zauth_cost = (zPER_EMPTY_ACCOUNT * z2zE1616);
    }
  }
  uint64_t zcommon;
  {
    uint64_t z2zE1615;
    {
      uint64_t z2zE1614;
      {
        uint64_t z2zE1613;
        {    z2zE1613 = (zdata_cost + zG_transaction);
        }
        {    z2zE1614 = (z2zE1613 + zaddress_cost);
        }
      }
      {    z2zE1615 = (z2zE1614 + zslot_cost);
      }
    }
    {    zcommon = (z2zE1615 + zauth_cost);
    }
  }
  bool z2zE1610;
  z2zE1610 = ztx.zis_create;
  if (z2zE1610) {
    uint64_t z2zE1611;
    {    z2zE1611 = (zcommon + zG_txcreate);
    }
    uint64_t z2zE1612;
    z2zE1612 = ztransaction_initcode_gas(zinput_len);
    {    z8zE493 = (z2zE1611 + z2zE1612);
    }
  } else {  z8zE493 = zcommon;  }
end_function_1934: ;
  return z8zE493;
end_block_exception_1935: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zlegacy_calldata_floor(struct zByteSliceFields zinput)
{
  uint64_t z8zE494;
  uint64_t znonzzeroes;
  znonzzeroes = zslice_count_nonzzero(zinput);
  uint64_t zinput_len;
  zinput_len = zinput.zlen;
  bool z2zE1605;
  z2zE1605 = (!(zinput_len < znonzzeroes));
  if (z2zE1605) {
    uint64_t zzzeroes;
    {    zzzeroes = (zinput_len - znonzzeroes);
    }
    uint64_t z2zE1608;
    {
      uint64_t z2zE1606;
      {    z2zE1606 = (UINT64_C(10) * zzzeroes);
      }
      uint64_t z2zE1607;
      {    z2zE1607 = (UINT64_C(40) * znonzzeroes);
      }
      {    z2zE1608 = (z2zE1606 + z2zE1607);
      }
    }
    {    z8zE494 = (z2zE1608 + zG_transaction);
    }
  } else {
    struct zexception z2zE1609;
    CREATE(zexception)(&z2zE1609);
    zInvalidBlock(&z2zE1609, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE1609);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/transaction.sail:187.8-187.44");
    KILL(zexception)(&z2zE1609);
    goto end_block_exception_1933;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1609);
  }
end_function_1932: ;
  return z8zE494;
end_block_exception_1933: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zamsterdam_recipient_execution_cost(struct zTransaction ztx)
{
  uint64_t z8zE495;
  bool z2zE1596;
  z2zE1596 = ztx.zis_create;
  if (z2zE1596) {
    bool z2zE1598;
    {
      sail_u256 z2zE1597;
      z2zE1597 = ztx.zvalue;
      z2zE1598 = zword_nonzzero(z2zE1597);
    }
    if (z2zE1598) {
      {    z8zE495 = (zAMSTERDAM_CREATE_ACCESS + zAMSTERDAM_TRANSFER_LOG_COST);
      }
    } else {  z8zE495 = zAMSTERDAM_CREATE_ACCESS;  }
  } else {
    bool z2zE1601;
    {
      sail_fixed_bytes_20 z2zE1599;
      z2zE1599 = ztx.zrecipient;
      sail_fixed_bytes_20 z2zE1600;
      z2zE1600 = ztx.zsender;
      z2zE1601 = zneq_anythingzIR__sail_c_repr_fixed_byteszIC20zKzK(z2zE1599, z2zE1600);
    }
    if (z2zE1601) {
      bool z2zE1603;
      {
        sail_u256 z2zE1602;
        z2zE1602 = ztx.zvalue;
        z2zE1603 = zword_nonzzero(z2zE1602);
      }
      if (z2zE1603) {
        uint64_t z2zE1604;
        {    z2zE1604 = (zAMSTERDAM_COLD_ACCOUNT_ACCESS + zAMSTERDAM_TX_VALUE_COST);
        }
        {    z8zE495 = (z2zE1604 + zAMSTERDAM_TRANSFER_LOG_COST);
        }
      } else {  z8zE495 = zAMSTERDAM_COLD_ACCOUNT_ACCESS;  }
    } else {  z8zE495 = UINT64_C(0);  }
  }
end_function_1930: ;
  return z8zE495;
end_block_exception_1931: ;

  return UINT64_C(0xdeadc0de);
}

struct zIntrinsicGasCost zintrinsic_gas(struct zTransaction ztx)
{
  struct zIntrinsicGasCost z8zE496;
  bool z2zE1571;
  z2zE1571 = zfork_lt(zk_fork, zAmsterdam);
  if (z2zE1571) {
    uint64_t z2zE1572;
    {
      z2zE1572 = zlegacy_intrinsic_gas(ztx);
      if (have_exception) {  goto end_block_exception_1929;  }
    }
    uint64_t z2zE1574;
    {
      struct zByteSliceFields z2zE1573;
      z2zE1573 = ztx.zinput_src;
      {
        z2zE1574 = zlegacy_calldata_floor(z2zE1573);
        if (have_exception) {  goto end_block_exception_1929;  }
      }
    }
    struct zIntrinsicGasCost z3zE1518;
    z3zE1518.zcalldata_floor = z2zE1574;
    z3zE1518.zexecution = z2zE1572;
    z3zE1518.zstate = UINT64_C(0);
    z8zE496 = z3zE1518;
  } else {
    struct zByteSliceFields zinput;
    zinput = ztx.zinput_src;
    uint64_t zrecipient;
    zrecipient = zamsterdam_recipient_execution_cost(ztx);
    uint64_t zaddress_count;
    zaddress_count = ztx.zaccess_list_address_count;
    uint64_t zslot_count;
    zslot_count = ztx.zaccess_list_slot_count;
    uint64_t zaccess_execution;
    {
      uint64_t z2zE1594;
      {
        uint64_t z2zE1592;
        {
          uint64_t z2zE1590;
          {    z2zE1590 = (zAMSTERDAM_ACCESS_LIST_ADDRESS * zaddress_count);
          }
          uint64_t z2zE1591;
          {    z2zE1591 = (zAMSTERDAM_ACCESS_LIST_SLOT * zslot_count);
          }
          {    z2zE1592 = (z2zE1590 + z2zE1591);
          }
        }
        uint64_t z2zE1593;
        {    z2zE1593 = (zAMSTERDAM_ACCESS_LIST_ADDRESS_FLOOR * zaddress_count);
        }
        {    z2zE1594 = (z2zE1592 + z2zE1593);
        }
      }
      uint64_t z2zE1595;
      {    z2zE1595 = (zAMSTERDAM_ACCESS_LIST_SLOT_FLOOR * zslot_count);
      }
      {    zaccess_execution = (z2zE1594 + z2zE1595);
      }
    }
    uint64_t zauthorizzation_execution;
    {
      uint64_t z2zE1589;
      z2zE1589 = ztx.zauthorizzation_count;
      {    zauthorizzation_execution = (zAMSTERDAM_AUTH_BASE * z2zE1589);
      }
    }
    uint64_t zcreate_execution;
    {
      bool z2zE1587;
      z2zE1587 = ztx.zis_create;
      if (z2zE1587) {
        uint64_t z2zE1588;
        z2zE1588 = zinput.zlen;
        zcreate_execution = ztransaction_initcode_gas(z2zE1588);
      } else {  zcreate_execution = UINT64_C(0);  }
    }
    uint64_t zexecution;
    {
      uint64_t z2zE1586;
      {
        uint64_t z2zE1585;
        {
          uint64_t z2zE1584;
          {
            uint64_t z2zE1583;
            {
              uint64_t z2zE1582;
              {
                struct zByteSliceFields z2zE1581;
                z2zE1581 = ztx.zinput_src;
                {
                  z2zE1582 = zcalldata_cost(z2zE1581);
                  if (have_exception) {  goto end_block_exception_1929;  }
                }
              }
              {    z2zE1583 = (z2zE1582 + zAMSTERDAM_TX_BASE);
              }
            }
            {    z2zE1584 = (z2zE1583 + zrecipient);
            }
          }
          {    z2zE1585 = (z2zE1584 + zaccess_execution);
          }
        }
        {    z2zE1586 = (z2zE1585 + zauthorizzation_execution);
        }
      }
      {    zexecution = (z2zE1586 + zcreate_execution);
      }
    }
    uint64_t zinput_length;
    zinput_length = zinput.zlen;
    uint64_t zfloor;
    {
      uint64_t z2zE1579;
      {
        uint64_t z2zE1577;
        {
          uint64_t z2zE1576;
          {
            uint64_t z2zE1575;
            {    z2zE1575 = (zAMSTERDAM_CALLDATA_FLOOR_BYTE * zinput_length);
            }
            {    z2zE1576 = (z2zE1575 + zAMSTERDAM_TX_BASE);
            }
          }
          {    z2zE1577 = (z2zE1576 + zrecipient);
          }
        }
        uint64_t z2zE1578;
        {    z2zE1578 = (zAMSTERDAM_ACCESS_LIST_ADDRESS_FLOOR * zaddress_count);
        }
        {    z2zE1579 = (z2zE1577 + z2zE1578);
        }
      }
      uint64_t z2zE1580;
      {    z2zE1580 = (zAMSTERDAM_ACCESS_LIST_SLOT_FLOOR * zslot_count);
      }
      {    zfloor = (z2zE1579 + z2zE1580);
      }
    }
    struct zIntrinsicGasCost z3zE1517;
    z3zE1517.zcalldata_floor = zfloor;
    z3zE1517.zexecution = zexecution;
    z3zE1517.zstate = UINT64_C(0);
    z8zE496 = z3zE1517;
  }
end_function_1928: ;
  return z8zE496;
end_block_exception_1929: ;
  struct zIntrinsicGasCost z8zE886 = { .zcalldata_floor = UINT64_C(0xdeadc0de), .zexecution = UINT64_C(0xdeadc0de), .zstate = UINT64_C(0xdeadc0de) };
  return z8zE886;
}

uint64_t zmax_blobs_per_transaction(unit z3zE1516)
{
  uint64_t z8zE497;
  bool z2zE1569;
  z2zE1569 = zfork_gteq(zk_fork, zOsaka);
  if (z2zE1569) {  z8zE497 = UINT64_C(6);  } else {
    bool z2zE1570;
    z2zE1570 = zfork_gteq(zk_fork, zPrague);
    if (z2zE1570) {  z8zE497 = UINT64_C(9);  } else {  z8zE497 = UINT64_C(6);  }
  }
end_function_1926: ;
  return z8zE497;
end_block_exception_1927: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t ztransaction_blob_gas_for_count(uint64_t zcount)
{
  uint64_t z8zE498;
  uint64_t zactive_maximum;
  zactive_maximum = zmax_blobs_per_transaction(UNIT);
  bool z2zE1567;
  {
    bool z2zE1566;
    z2zE1566 = (!(UINT64_C(9) < zactive_maximum));
    bool z3zE1515;
    if (z2zE1566) {  z3zE1515 = (!(zactive_maximum < zcount));  } else {  z3zE1515 = false;  }
    z2zE1567 = z3zE1515;
  }
  if (z2zE1567) {
    {    z8zE498 = (UINT64_C(131072) * zcount);
    }
  } else {
    struct zexception z2zE1568;
    CREATE(zexception)(&z2zE1568);
    zInvalidBlock(&z2zE1568, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE1568);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/transaction.sail:275.8-275.44");
    KILL(zexception)(&z2zE1568);
    goto end_block_exception_1925;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1568);
  }
end_function_1924: ;
  return z8zE498;
end_block_exception_1925: ;

  return UINT64_C(0xdeadc0de);
}

struct zTransactionCosts ztransaction_costs(struct zTransaction ztx, uint64_t zgas_limit, sail_u256 zblob_price)
{
  struct zTransactionCosts z8zE499;
  struct zIntrinsicGasCost zintrinsic;
  {
    zintrinsic = zintrinsic_gas(ztx);
    if (have_exception) {  goto end_block_exception_1923;  }
  }
  uint64_t zblob_gas;
  {
    uint64_t z2zE1565;
    {
      struct zBlobHashes z2zE1564;
      z2zE1564 = ztx.zblob_hashes;
      z2zE1565 = z2zE1564.zcount;
    }
    {
      zblob_gas = ztransaction_blob_gas_for_count(z2zE1565);
      if (have_exception) {  goto end_block_exception_1923;  }
    }
  }
  sail_u256 zblob_fee;
  {
    zblob_fee = zblob_word_mulzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zblob_price, zblob_gas);
    if (have_exception) {  goto end_block_exception_1923;  }
  }
  sail_u256 zexecution_cap;
  {
    sail_u256 z2zE1563;
    z2zE1563 = ztx.zmax_fee;
    {
      zexecution_cap = zblob_word_mulzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE1563, zgas_limit);
      if (have_exception) {  goto end_block_exception_1923;  }
    }
  }
  sail_u256 zblob_cap;
  {
    sail_u256 z2zE1562;
    z2zE1562 = ztx.zmax_blob_fee;
    {
      zblob_cap = zblob_word_mulzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE1562, zblob_gas);
      if (have_exception) {  goto end_block_exception_1923;  }
    }
  }
  sail_u256 zupfront;
  {
    sail_u256 z2zE1561;
    {
      sail_u256 z2zE1560;
      z2zE1560 = ztx.zvalue;
      {
        z2zE1561 = zblob_word_add(zexecution_cap, z2zE1560);
        if (have_exception) {  goto end_block_exception_1923;  }
      }
    }
    {
      zupfront = zblob_word_add(z2zE1561, zblob_cap);
      if (have_exception) {  goto end_block_exception_1923;  }
    }
  }
  uint64_t z2zE1557;
  z2zE1557 = zintrinsic.zexecution;
  uint64_t z2zE1558;
  z2zE1558 = zintrinsic.zstate;
  uint64_t z2zE1559;
  z2zE1559 = zintrinsic.zcalldata_floor;
  struct zTransactionCosts z3zE1514;
  z3zE1514.zblob_fee = zblob_fee;
  z3zE1514.zblob_gas = zblob_gas;
  z3zE1514.zcalldata_floor = z2zE1559;
  z3zE1514.zintrinsic_execution = z2zE1557;
  z3zE1514.zintrinsic_state = z2zE1558;
  z3zE1514.zupfront = zupfront;
  z8zE499 = z3zE1514;
end_function_1922: ;
  return z8zE499;
end_block_exception_1923: ;
  struct zTransactionCosts z8zE887 = { .zblob_fee = u256_zero(), .zblob_gas = UINT64_C(0xdeadc0de), .zcalldata_floor = UINT64_C(0xdeadc0de), .zintrinsic_execution = UINT64_C(0xdeadc0de), .zintrinsic_state = UINT64_C(0xdeadc0de), .zupfront = u256_zero() };
  return z8zE887;
}

uint64_t zvalidated_gas_add(uint64_t zleft_gas, uint64_t zright_gas)
{
  uint64_t z8zE500;
  {
    z8zE500 = zconserved_gas_add(zleft_gas, zright_gas);
    if (have_exception) {  goto end_block_exception_1919;  }
  }
end_function_1918: ;
  return z8zE500;
end_block_exception_1919: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zadmitted_transaction_gas_limit(uint64_t zvalue, uint64_t zblock_limit)
{
  uint64_t z8zE501;
  bool z2zE1553;
  z2zE1553 = (zblock_limit < zvalue);
  if (z2zE1553) {
    struct zexception z2zE1554;
    CREATE(zexception)(&z2zE1554);
    zInvalidBlock(&z2zE1554, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE1554);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/transaction.sail:323.8-323.44");
    KILL(zexception)(&z2zE1554);
    goto end_block_exception_1915;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1554);
  } else {  z8zE501 = zvalue;  }
end_function_1914: ;
  return z8zE501;
end_block_exception_1915: ;

  return UINT64_C(0xdeadc0de);
}

__int128 zprocess_auth(struct zAuthorizzation zau)
{
  __int128 z8zE502;
  __int128 zrefund;
  zrefund = zGAS_REFUND_ZERO;
  sail_fixed_bytes_20 zauthority;
  zauthority = zau.zauthority;
  bool z2zE1541;
  {
    bool z2zE1540;
    z2zE1540 = zau.zvalid_sig;
    bool z3zE1502;
    if (z2zE1540) {
      bool z2zE1539;
      {
        sail_u256 z2zE1536;
        z2zE1536 = zau.zchain_id;
        z2zE1539 = zword_is_zzero(z2zE1536);
      }
      bool z3zE1501;
      if (z2zE1539) {  z3zE1501 = true;  } else {
        sail_u256 z2zE1537;
        z2zE1537 = zau.zchain_id;
        uint64_t z2zE1538;
        z2zE1538 = zword_of_chain_identifier(zk_chain_id);
        z3zE1501 = u256_eq_u64(z2zE1537, z2zE1538);
      }
      z3zE1502 = z3zE1501;
    } else {  z3zE1502 = false;  }
    z2zE1541 = z3zE1502;
  }
  unit z3zE1503;
  if (z2zE1541) {
    bool z2zE1542;
    z2zE1542 = zk_access_account(zauthority);
    unit z3zE1504;
    {
      struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1543;
      {
        z2zE1543 = zk_deleg_target(zauthority);
        if (have_exception) {  goto end_block_exception_1913;  }
      }
      unit z3zE1505;
      {
        bool zis_deleg;
        zis_deleg = z2zE1543.ztup0;
        bool z2zE1549;
        {
          bool z2zE1548;
          {
            bool z2zE1545;
            {
              sail_fixed_bytes_32 z2zE1544;
              {
                z2zE1544 = zk_code_key(zauthority);
                if (have_exception) {  goto end_block_exception_1913;  }
              }
              z2zE1545 = eq_fixed_bytes_32(z2zE1544, zKECCAK_EMPTY);
            }
            bool z3zE1506;
            if (z2zE1545) {  z3zE1506 = true;  } else {  z3zE1506 = zis_deleg;  }
            z2zE1548 = z3zE1506;
          }
          bool z3zE1507;
          if (z2zE1548) {
            uint64_t z2zE1546;
            {
              z2zE1546 = zk_get_nonce(zauthority);
              if (have_exception) {  goto end_block_exception_1913;  }
            }
            uint64_t z2zE1547;
            z2zE1547 = zau.znonce;
            z3zE1507 = (z2zE1546 == z2zE1547);
          } else {  z3zE1507 = false;  }
          z2zE1549 = z3zE1507;
        }
        if (z2zE1549) {
          bool zexisted;
          {
            zexisted = zk_account_exists(zauthority);
            if (have_exception) {  goto end_block_exception_1913;  }
          }
          bool z2zE1551;
          {
            sail_fixed_bytes_20 z2zE1550;
            z2zE1550 = zau.zaddress;
            z2zE1551 = eq_fixed_bytes_20(z2zE1550, zZERO_ADDRESS);
          }
          unit z3zE1510;
          if (z2zE1551) {
            {
              z3zE1510 = zk_clear_code(zauthority);
              if (have_exception) {  goto end_block_exception_1913;  }
            }
          } else {
            sail_fixed_bytes_20 z2zE1552;
            z2zE1552 = zau.zaddress;
            {
              z3zE1510 = zk_set_delegation(zauthority, z2zE1552);
              if (have_exception) {  goto end_block_exception_1913;  }
            }
          }
          unit z3zE1509;
          {
            z3zE1509 = zk_bump_nonce(zauthority);
            if (have_exception) {  goto end_block_exception_1913;  }
          }
          if (zexisted) {
            {
              __int128 z3zE1511;
              z3zE1511 = (__int128)(zPER_EMPTY_ACCOUNT);
              __int128 z3zE1512;
              z3zE1512 = (__int128)(zPER_AUTH_BASE);
              zrefund = (z3zE1511 - z3zE1512);
            }
            z3zE1505 = UNIT;
          } else {  z3zE1505 = UNIT;  }
        } else {  z3zE1505 = UNIT;  }
        goto finish_match_1910;
      }
    case_1911: ;
      sail_match_failure("process_auth");
    finish_match_1910: ;
      z3zE1504 = z3zE1505;
      goto finish_match_1908;
    }
  case_1909: ;
    sail_match_failure("process_auth");
  finish_match_1908: ;
    z3zE1503 = z3zE1504;
  } else {  z3zE1503 = UNIT;  }
  z8zE502 = zrefund;
end_function_1912: ;
  return z8zE502;
end_block_exception_1913: ;

  return ((__int128)INT64_C(0xdeadc0de));
}

__int128 zprocess_auth_list(zz5listz8z5structz0zzAuthorizzzzationz9 zxs)
{
  __int128 z8zE503;
  __int128 z3zE1498;
  {
    if (!((zxs == NULL))) goto case_1905;
    z3zE1498 = zGAS_REFUND_ZERO;
    goto finish_match_1903;
  }
case_1905: ;
  {
    /* complete */
    struct zAuthorizzation za;
    za = (*zxs).hd;
    zz5listz8z5structz0zzAuthorizzzzationz9 zr;
    CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&zr);
    COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&zr, (*zxs).tl);
    __int128 z2zE1534;
    {
      z2zE1534 = zprocess_auth(za);
      if (have_exception) {
        KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zr);
        goto end_block_exception_1907;
      }
    }
    __int128 z2zE1535;
    {
      z2zE1535 = zprocess_auth_list(zr);
      if (have_exception) {
        KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zr);
        goto end_block_exception_1907;
      }
    }
    {
      z3zE1498 = zvalidated_refund_add(z2zE1534, z2zE1535);
      if (have_exception) {
        KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zr);
        goto end_block_exception_1907;
      }
    }
    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zr);
    goto finish_match_1903;
  }
case_1904: ;
finish_match_1903: ;
  z8zE503 = z3zE1498;
end_function_1906: ;
  return z8zE503;
end_block_exception_1907: ;

  return ((__int128)INT64_C(0xdeadc0de));
}

bool zauthorizzation_address_seen(sail_fixed_bytes_20 za, zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 zxs)
{
  bool z8zE504;
  bool z3zE1494;
  {
    if (!((zxs == NULL))) goto case_1900;
    z3zE1494 = false;
    goto finish_match_1898;
  }
case_1900: ;
  {
    /* complete */
    sail_fixed_bytes_20 zh;
    zh = (*zxs).hd;
    zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 zt;
    CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zt);
    COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zt, (*zxs).tl);
    bool z2zE1533;
    z2zE1533 = eq_fixed_bytes_20(za, zh);
    bool z3zE1495;
    if (z2zE1533) {  z3zE1495 = true;  } else {  z3zE1495 = zauthorizzation_address_seen(za, zt);  }
    z3zE1494 = z3zE1495;
    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zt);
    goto finish_match_1898;
  }
case_1899: ;
finish_match_1898: ;
  z8zE504 = z3zE1494;
end_function_1901: ;
  return z8zE504;
end_block_exception_1902: ;

  return false;
}

void zprocess_amsterdam_auth(struct zAmsterdamAuthorizzationState *z8zE505, struct zAuthorizzation zau, sail_fixed_bytes_20 zsender, sail_fixed_bytes_20 zcurrent_target, bool ztransfers_value, struct zAmsterdamAuthorizzationState zauth_state)
{
  struct zAmsterdamAuthorizzationState znext;
  CREATE(zAmsterdamAuthorizzationState)(&znext);
  COPY(zAmsterdamAuthorizzationState)(&znext, zauth_state);
  sail_fixed_bytes_20 zauthority;
  zauthority = zau.zauthority;
  bool z2zE1493;
  {
    bool z2zE1492;
    z2zE1492 = zau.zvalid_sig;
    bool z3zE1467;
    if (z2zE1492) {
      bool z2zE1491;
      {
        sail_u256 z2zE1488;
        z2zE1488 = zau.zchain_id;
        z2zE1491 = zword_is_zzero(z2zE1488);
      }
      bool z3zE1466;
      if (z2zE1491) {  z3zE1466 = true;  } else {
        sail_u256 z2zE1489;
        z2zE1489 = zau.zchain_id;
        uint64_t z2zE1490;
        z2zE1490 = zword_of_chain_identifier(zk_chain_id);
        z3zE1466 = u256_eq_u64(z2zE1489, z2zE1490);
      }
      z3zE1467 = z3zE1466;
    } else {  z3zE1467 = false;  }
    z2zE1493 = z3zE1467;
  }
  unit z3zE1468;
  if (z2zE1493) {
    bool z2zE1494;
    z2zE1494 = zk_access_account(zauthority);
    unit z3zE1469;
    {
      struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1495;
      {
        z2zE1495 = zk_deleg_target(zauthority);
        if (have_exception) {
          KILL(zAmsterdamAuthorizzationState)(&znext);
          goto end_block_exception_1897;
        }
      }
      unit z3zE1470;
      {
        bool zcurrently_delegated;
        zcurrently_delegated = z2zE1495.ztup0;
        bool z2zE1501;
        {
          bool z2zE1500;
          {
            bool z2zE1497;
            {
              sail_fixed_bytes_32 z2zE1496;
              {
                z2zE1496 = zk_code_key(zauthority);
                if (have_exception) {
                  KILL(zAmsterdamAuthorizzationState)(&znext);
                  goto end_block_exception_1897;
                }
              }
              z2zE1497 = eq_fixed_bytes_32(z2zE1496, zKECCAK_EMPTY);
            }
            bool z3zE1471;
            if (z2zE1497) {  z3zE1471 = true;  } else {  z3zE1471 = zcurrently_delegated;  }
            z2zE1500 = z3zE1471;
          }
          bool z3zE1472;
          if (z2zE1500) {
            uint64_t z2zE1498;
            {
              z2zE1498 = zk_get_nonce(zauthority);
              if (have_exception) {
                KILL(zAmsterdamAuthorizzationState)(&znext);
                goto end_block_exception_1897;
              }
            }
            uint64_t z2zE1499;
            z2zE1499 = zau.znonce;
            z3zE1472 = (z2zE1498 == z2zE1499);
          } else {  z3zE1472 = false;  }
          z2zE1501 = z3zE1472;
        }
        if (z2zE1501) {
          bool zseen;
          {
            zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1532;
            CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1532);
            COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1532, zauth_state.zseen_valid_authorities);
            zseen = zauthorizzation_address_seen(zauthority, z2zE1532);
            KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1532);
          }
          bool zdelegated_before_tx;
          if (zseen) {
            zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1531;
            CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1531);
            COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1531, zauth_state.zoriginally_delegated);
            zdelegated_before_tx = zauthorizzation_address_seen(zauthority, z2zE1531);
            KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1531);
          } else {  zdelegated_before_tx = zcurrently_delegated;  }
          bool zalready_written;
          {
            bool z3zE1476;
            if (zseen) {  z3zE1476 = true;  } else {
              bool z2zE1530;
              z2zE1530 = eq_fixed_bytes_20(zauthority, zsender);
              bool z3zE1475;
              if (z2zE1530) {  z3zE1475 = true;  } else {
                bool z3zE1474;
                if (ztransfers_value) {  z3zE1474 = eq_fixed_bytes_20(zauthority, zcurrent_target);  } else {
                  z3zE1474 = false;
                }
                z3zE1475 = z3zE1474;
              }
              z3zE1476 = z3zE1475;
            }
            zalready_written = z3zE1476;
          }
          bool z2zE1503;
          {
            bool z2zE1502;
            {
              z2zE1502 = zk_account_exists(zauthority);
              if (have_exception) {
                KILL(zAmsterdamAuthorizzationState)(&znext);
                goto end_block_exception_1897;
              }
            }
            z2zE1503 = not(z2zE1502);
          }
          unit z3zE1483;
          if (z2zE1503) {
            {
              z3zE1483 = zcharge_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_amsterdam_state_new_account);
              if (have_exception) {
                KILL(zAmsterdamAuthorizzationState)(&znext);
                goto end_block_exception_1897;
              }
            }
          } else {  z3zE1483 = UNIT;  }
          bool z2zE1505;
          {
            bool z2zE1504;
            z2zE1504 = zis_running(UNIT);
            bool z3zE1477;
            if (z2zE1504) {  z3zE1477 = not(zalready_written);  } else {  z3zE1477 = false;  }
            z2zE1505 = z3zE1477;
          }
          unit z3zE1482;
          if (z2zE1505) {
            {
              z3zE1482 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_amsterdam_account_write);
              if (have_exception) {
                KILL(zAmsterdamAuthorizzationState)(&znext);
                goto end_block_exception_1897;
              }
            }
          } else {  z3zE1482 = UNIT;  }
          bool z2zE1512;
          {
            bool z2zE1511;
            z2zE1511 = zis_running(UNIT);
            bool z3zE1480;
            if (z2zE1511) {
              bool z2zE1510;
              {
                sail_fixed_bytes_20 z2zE1506;
                z2zE1506 = zau.zaddress;
                z2zE1510 = zneq_anythingzIR__sail_c_repr_fixed_byteszIC20zKzK(z2zE1506, zZERO_ADDRESS);
              }
              bool z3zE1479;
              if (z2zE1510) {
                bool z2zE1509;
                z2zE1509 = not(zdelegated_before_tx);
                bool z3zE1478;
                if (z2zE1509) {
                  bool z2zE1508;
                  {
                    zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1507;
                    CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1507);
                    COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1507, zauth_state.zdelegation_set_for);
                    z2zE1508 = zauthorizzation_address_seen(zauthority, z2zE1507);
                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1507);
                  }
                  z3zE1478 = not(z2zE1508);
                } else {  z3zE1478 = false;  }
                z3zE1479 = z3zE1478;
              } else {  z3zE1479 = false;  }
              z3zE1480 = z3zE1479;
            } else {  z3zE1480 = false;  }
            z2zE1512 = z3zE1480;
          }
          unit z3zE1481;
          if (z2zE1512) {
            {
              z3zE1481 = zcharge_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_amsterdam_state_auth_base);
              if (have_exception) {
                KILL(zAmsterdamAuthorizzationState)(&znext);
                goto end_block_exception_1897;
              }
            }
          } else {  z3zE1481 = UNIT;  }
          bool z2zE1513;
          z2zE1513 = zis_running(UNIT);
          if (z2zE1513) {
            bool z2zE1515;
            {
              sail_fixed_bytes_20 z2zE1514;
              z2zE1514 = zau.zaddress;
              z2zE1515 = eq_fixed_bytes_20(z2zE1514, zZERO_ADDRESS);
            }
            unit z3zE1490;
            if (z2zE1515) {
              {
                z3zE1490 = zk_clear_code(zauthority);
                if (have_exception) {
                  KILL(zAmsterdamAuthorizzationState)(&znext);
                  goto end_block_exception_1897;
                }
              }
            } else {
              sail_fixed_bytes_20 z2zE1516;
              z2zE1516 = zau.zaddress;
              {
                z3zE1490 = zk_set_delegation(zauthority, z2zE1516);
                if (have_exception) {
                  KILL(zAmsterdamAuthorizzationState)(&znext);
                  goto end_block_exception_1897;
                }
              }
            }
            unit z3zE1489;
            {
              z3zE1489 = zk_bump_nonce(zauthority);
              if (have_exception) {
                KILL(zAmsterdamAuthorizzationState)(&znext);
                goto end_block_exception_1897;
              }
            }
            zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1518;
            CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1518);
            {
              zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1517;
              CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1517);
              COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1517, znext.zseen_valid_authorities);
              zconsz3z5structz0zz__sail_c_repr_fixed_bytesz820z9(&z2zE1518, zauthority, z2zE1517);
              KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1517);
            }
            COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&znext)->zseen_valid_authorities), z2zE1518);
            unit z3zE1488;
            z3zE1488 = UNIT;
            bool z2zE1520;
            {
              bool z2zE1519;
              z2zE1519 = not(zseen);
              bool z3zE1485;
              if (z2zE1519) {  z3zE1485 = zcurrently_delegated;  } else {  z3zE1485 = false;  }
              z2zE1520 = z3zE1485;
            }
            unit z3zE1486;
            if (z2zE1520) {
              zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1522;
              CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1522);
              {
                zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1521;
                CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1521);
                COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1521, znext.zoriginally_delegated);
                zconsz3z5structz0zz__sail_c_repr_fixed_bytesz820z9(&z2zE1522, zauthority, z2zE1521);
              }
              struct zAmsterdamAuthorizzationState z3zE1487;
              CREATE(zAmsterdamAuthorizzationState)(&z3zE1487);
              COPY(zAmsterdamAuthorizzationState)(&z3zE1487, znext);
              COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE1487)->zoriginally_delegated), z2zE1522);
              COPY(zAmsterdamAuthorizzationState)(&znext, z3zE1487);
              z3zE1486 = UNIT;
            } else {  z3zE1486 = UNIT;  }
            bool z2zE1527;
            {
              bool z2zE1526;
              {
                sail_fixed_bytes_20 z2zE1523;
                z2zE1523 = zau.zaddress;
                z2zE1526 = zneq_anythingzIR__sail_c_repr_fixed_byteszIC20zKzK(z2zE1523, zZERO_ADDRESS);
              }
              bool z3zE1491;
              if (z2zE1526) {
                bool z2zE1525;
                {
                  zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1524;
                  CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1524);
                  COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1524, znext.zdelegation_set_for);
                  z2zE1525 = zauthorizzation_address_seen(zauthority, z2zE1524);
                }
                z3zE1491 = not(z2zE1525);
              } else {  z3zE1491 = false;  }
              z2zE1527 = z3zE1491;
            }
            if (z2zE1527) {
              zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1529;
              CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1529);
              {
                zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1528;
                CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1528);
                COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1528, znext.zdelegation_set_for);
                zconsz3z5structz0zz__sail_c_repr_fixed_bytesz820z9(&z2zE1529, zauthority, z2zE1528);
              }
              struct zAmsterdamAuthorizzationState z3zE1492;
              CREATE(zAmsterdamAuthorizzationState)(&z3zE1492);
              COPY(zAmsterdamAuthorizzationState)(&z3zE1492, znext);
              COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE1492)->zdelegation_set_for), z2zE1529);
              COPY(zAmsterdamAuthorizzationState)(&znext, z3zE1492);
              z3zE1470 = UNIT;
            } else {  z3zE1470 = UNIT;  }
          } else {  z3zE1470 = UNIT;  }
        } else {  z3zE1470 = UNIT;  }
        goto finish_match_1894;
      }
    case_1895: ;
      sail_match_failure("process_amsterdam_auth");
    finish_match_1894: ;
      z3zE1469 = z3zE1470;
      goto finish_match_1892;
    }
  case_1893: ;
    sail_match_failure("process_amsterdam_auth");
  finish_match_1892: ;
    z3zE1468 = z3zE1469;
  } else {  z3zE1468 = UNIT;  }
  COPY(zAmsterdamAuthorizzationState)((*(&z8zE505)), znext);
  KILL(zAmsterdamAuthorizzationState)(&znext);
end_function_1896: ;
  goto end_function_3634;
end_block_exception_1897: ;
  goto end_function_3634;
end_function_3634: ;
}

void zprocess_amsterdam_auth_list(struct zAmsterdamAuthorizzationState *z8zE506, zz5listz8z5structz0zzAuthorizzzzationz9 zxs, sail_fixed_bytes_20 zsender, sail_fixed_bytes_20 zcurrent_target, bool ztransfers_value, struct zAmsterdamAuthorizzationState zauth_state)
{
  struct zAmsterdamAuthorizzationState z3zE1463;
  CREATE(zAmsterdamAuthorizzationState)(&z3zE1463);
  {
    if (!((zxs == NULL))) goto case_1889;
    COPY(zAmsterdamAuthorizzationState)(&z3zE1463, zauth_state);
    goto finish_match_1887;
  }
case_1889: ;
  {
    /* complete */
    struct zAuthorizzation za;
    za = (*zxs).hd;
    zz5listz8z5structz0zzAuthorizzzzationz9 zr;
    CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&zr);
    COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&zr, (*zxs).tl);
    struct zAmsterdamAuthorizzationState znext;
    CREATE(zAmsterdamAuthorizzationState)(&znext);
    {
      zprocess_amsterdam_auth(&znext, za, zsender, zcurrent_target, ztransfers_value, zauth_state);
      if (have_exception) {
        KILL(zAmsterdamAuthorizzationState)(&z3zE1463);
        KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zr);
        KILL(zAmsterdamAuthorizzationState)(&znext);
        goto end_block_exception_1891;
      }
    }
    bool z2zE1487;
    z2zE1487 = zis_running(UNIT);
    if (z2zE1487) {
      {
        zprocess_amsterdam_auth_list(&z3zE1463, zr, zsender, zcurrent_target, ztransfers_value, znext);
        if (have_exception) {
          KILL(zAmsterdamAuthorizzationState)(&z3zE1463);
          KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zr);
          KILL(zAmsterdamAuthorizzationState)(&znext);
          goto end_block_exception_1891;
        }
      }
    } else {  COPY(zAmsterdamAuthorizzationState)(&z3zE1463, znext);  }
    KILL(zAmsterdamAuthorizzationState)(&znext);
    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zr);
    goto finish_match_1887;
  }
case_1888: ;
finish_match_1887: ;
  COPY(zAmsterdamAuthorizzationState)((*(&z8zE506)), z3zE1463);
  KILL(zAmsterdamAuthorizzationState)(&z3zE1463);
end_function_1890: ;
  goto end_function_3633;
end_block_exception_1891: ;
  goto end_function_3633;
end_function_3633: ;
}

unit zwarm_access_list_addresses(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 zxs)
{
  unit z8zE507;
  unit z3zE1458;
  {
    if (!((zxs == NULL))) goto case_1884;
    z3zE1458 = UNIT;
    goto finish_match_1880;
  }
case_1884: ;
  {
    /* complete */
    sail_fixed_bytes_20 za;
    za = (*zxs).hd;
    zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 zr;
    CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zr);
    COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zr, (*zxs).tl);
    bool z2zE1486;
    z2zE1486 = zk_access_account(za);
    unit z3zE1459;
    {
      z3zE1459 = zwarm_access_list_addresses(zr);
      goto finish_match_1882;
    }
  case_1883: ;
    sail_match_failure("warm_access_list_addresses");
  finish_match_1882: ;
    z3zE1458 = z3zE1459;
    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zr);
    goto finish_match_1880;
  }
case_1881: ;
finish_match_1880: ;
  z8zE507 = z3zE1458;
end_function_1885: ;
  return z8zE507;
end_block_exception_1886: ;

  return UNIT;
}

unit zwarm_access_list_slots(zz5listz8z5structz0zzStorageKeyz9 zxs)
{
  unit z8zE508;
  unit z3zE1453;
  {
    if (!((zxs == NULL))) goto case_1877;
    z3zE1453 = UNIT;
    goto finish_match_1873;
  }
case_1877: ;
  {
    /* complete */
    struct zStorageKey zk;
    zk = (*zxs).hd;
    zz5listz8z5structz0zzStorageKeyz9 zr;
    CREATE(zz5listz8z5structz0zzStorageKeyz9)(&zr);
    COPY(zz5listz8z5structz0zzStorageKeyz9)(&zr, (*zxs).tl);
    bool z2zE1485;
    {
      sail_fixed_bytes_20 z2zE1483;
      z2zE1483 = zk.zaddr;
      sail_u256 z2zE1484;
      z2zE1484 = zk.zslot;
      z2zE1485 = zk_slot_is_warm(z2zE1483, z2zE1484);
    }
    unit z3zE1454;
    {
      z3zE1454 = zwarm_access_list_slots(zr);
      goto finish_match_1875;
    }
  case_1876: ;
    sail_match_failure("warm_access_list_slots");
  finish_match_1875: ;
    z3zE1453 = z3zE1454;
    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zr);
    goto finish_match_1873;
  }
case_1874: ;
finish_match_1873: ;
  z8zE508 = z3zE1453;
end_function_1878: ;
  return z8zE508;
end_block_exception_1879: ;

  return UNIT;
}

unit zprewarm(struct zTransaction ztx)
{
  unit z8zE509;
  bool z2zE1469;
  {
    sail_fixed_bytes_20 z2zE1468;
    z2zE1468 = ztx.zsender;
    z2zE1469 = zk_access_account(z2zE1468);
  }
  unit z3zE1433;
  {
    bool z2zE1472;
    {
      bool z2zE1470;
      z2zE1470 = ztx.zis_create;
      if (z2zE1470) {  z2zE1472 = false;  } else {
        sail_fixed_bytes_20 z2zE1471;
        z2zE1471 = ztx.zrecipient;
        z2zE1472 = zk_access_account(z2zE1471);
      }
    }
    unit z3zE1434;
    {
      bool z2zE1473;
      z2zE1473 = zfork_gteq(zk_fork, zShanghai);
      unit z3zE1435;
      if (z2zE1473) {
        bool z2zE1475;
        {
          sail_fixed_bytes_20 z2zE1474;
          z2zE1474 = zk_coinbase(UNIT);
          z2zE1475 = zk_access_account(z2zE1474);
        }
        unit z3zE1436;
        {
          z3zE1436 = UNIT;
          goto finish_match_1863;
        }
      case_1864: ;
        sail_match_failure("prewarm");
      finish_match_1863: ;
        z3zE1435 = z3zE1436;
      } else {  z3zE1435 = UNIT;  }
      zz5vecz8z5u64z9 zprecompile_addresses;
      CREATE(zz5vecz8z5u64z9)(&zprecompile_addresses);
      {
        zz5vecz8z5u64z9 z3zE1438;
        CREATE(zz5vecz8z5u64z9)(&z3zE1438);
        internal_vector_init_zz5vecz8z5u64z9(&z3zE1438, INT64_C(17));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1438, z3zE1438, INT64_C(0), UINT64_C(17));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1438, z3zE1438, INT64_C(1), UINT64_C(16));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1438, z3zE1438, INT64_C(2), UINT64_C(15));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1438, z3zE1438, INT64_C(3), UINT64_C(14));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1438, z3zE1438, INT64_C(4), UINT64_C(13));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1438, z3zE1438, INT64_C(5), UINT64_C(12));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1438, z3zE1438, INT64_C(6), UINT64_C(11));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1438, z3zE1438, INT64_C(7), UINT64_C(10));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1438, z3zE1438, INT64_C(8), UINT64_C(9));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1438, z3zE1438, INT64_C(9), UINT64_C(8));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1438, z3zE1438, INT64_C(10), UINT64_C(7));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1438, z3zE1438, INT64_C(11), UINT64_C(6));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1438, z3zE1438, INT64_C(12), UINT64_C(5));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1438, z3zE1438, INT64_C(13), UINT64_C(4));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1438, z3zE1438, INT64_C(14), UINT64_C(3));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1438, z3zE1438, INT64_C(15), UINT64_C(2));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1438, z3zE1438, INT64_C(16), UINT64_C(1));
        KILL(zz5vecz8z5u64z9)(&zprecompile_addresses);
        internal_vector_init_zz5vecz8z5u64z9(&zprecompile_addresses, z3zE1438.len);
        for (int z8zE888 = 0; z8zE888 < z3zE1438.len; z8zE888++) {
          uint64_t z8zE889 = z3zE1438.data[z8zE888];
          uint64_t z8zE890;
          z8zE890 = z8zE889;
          zprecompile_addresses.data[z8zE888] = z8zE890;
        }
        KILL(zz5vecz8z5u64z9)(&z3zE1438);
      }
      int64_t z3zE1439;
      {    z3zE1439 = (int64_t)(UINT64_C(0));
      }
      int64_t z3zE1440;
      {    z3zE1440 = (int64_t)(UINT64_C(16));
      }
      int64_t z3zE1441;
      {    z3zE1441 = (int64_t)(UINT64_C(1));
      }
      {
        int64_t zi;
        zi = z3zE1439;
        unit z3zE1442;
      for_start_1865: ;
        {
          if ((z3zE1440 < zi)) goto for_end_1866;
          uint64_t zp;
          zp = fast_vector_access_zz5vecz8z5u64z9(zprecompile_addresses, zi);
          bool z2zE1476;
          z2zE1476 = zprecompile_active_at_fork(zp);
          if (z2zE1476) {
            bool z2zE1478;
            {
              sail_fixed_bytes_20 z2zE1477;
              z2zE1477 = zprecompile_id_to_address(zp);
              z2zE1478 = zk_access_account(z2zE1477);
            }
            unit z3zE1443;
            {
              z3zE1443 = UNIT;
              goto finish_match_1867;
            }
          case_1868: ;
            sail_match_failure("prewarm");
          finish_match_1867: ;
            z3zE1442 = z3zE1443;
          } else {  z3zE1442 = UNIT;  }
          zi = (zi + z3zE1441);
          goto for_start_1865;
        }
      for_end_1866: ;
      }
      unit z3zE1450;
      z3zE1450 = UNIT;
      bool z2zE1479;
      z2zE1479 = zprecompile_active_at_fork(UINT64_C(256));
      unit z3zE1446;
      if (z2zE1479) {
        bool z2zE1480;
        {
          zz5vecz8z5bv8z9 z3zE1447;
          CREATE(zz5vecz8z5bv8z9)(&z3zE1447);
          internal_vector_init_zz5vecz8z5bv8z9(&z3zE1447, INT64_C(20));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1447, z3zE1447, INT64_C(0), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1447, z3zE1447, INT64_C(1), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1447, z3zE1447, INT64_C(2), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1447, z3zE1447, INT64_C(3), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1447, z3zE1447, INT64_C(4), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1447, z3zE1447, INT64_C(5), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1447, z3zE1447, INT64_C(6), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1447, z3zE1447, INT64_C(7), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1447, z3zE1447, INT64_C(8), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1447, z3zE1447, INT64_C(9), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1447, z3zE1447, INT64_C(10), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1447, z3zE1447, INT64_C(11), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1447, z3zE1447, INT64_C(12), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1447, z3zE1447, INT64_C(13), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1447, z3zE1447, INT64_C(14), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1447, z3zE1447, INT64_C(15), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1447, z3zE1447, INT64_C(16), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1447, z3zE1447, INT64_C(17), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1447, z3zE1447, INT64_C(18), UINT64_C(0x01));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1447, z3zE1447, INT64_C(19), UINT64_C(0x00));
          {
            sail_fixed_bytes_20 z3zE3047;
            for (size_t z8zE891 = 0; z8zE891 < 20; ++z8zE891) {
              z3zE3047.bytes[z8zE891] = (uint8_t)(z3zE1447.data[z8zE891] & UINT64_C(0xff));
            }
            z2zE1480 = zk_access_account(z3zE3047);
          }
          KILL(zz5vecz8z5bv8z9)(&z3zE1447);
        }
        unit z3zE1448;
        {
          z3zE1448 = UNIT;
          goto finish_match_1869;
        }
      case_1870: ;
        sail_match_failure("prewarm");
      finish_match_1869: ;
        z3zE1446 = z3zE1448;
      } else {  z3zE1446 = UNIT;  }
      zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1481;
      CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1481);
      COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1481, ztx.zaccess_list_addresses);
      unit z3zE1445;
      z3zE1445 = zwarm_access_list_addresses(z2zE1481);
      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1481);
      zz5listz8z5structz0zzStorageKeyz9 z2zE1482;
      CREATE(zz5listz8z5structz0zzStorageKeyz9)(&z2zE1482);
      COPY(zz5listz8z5structz0zzStorageKeyz9)(&z2zE1482, ztx.zaccess_list_slots);
      z3zE1434 = zwarm_access_list_slots(z2zE1482);
      KILL(zz5listz8z5structz0zzStorageKeyz9)(&z2zE1482);
      KILL(zz5vecz8z5u64z9)(&zprecompile_addresses);
      goto finish_match_1861;
    }
  case_1862: ;
    sail_match_failure("prewarm");
  finish_match_1861: ;
    z3zE1433 = z3zE1434;
    goto finish_match_1859;
  }
case_1860: ;
  sail_match_failure("prewarm");
finish_match_1859: ;
  z8zE509 = z3zE1433;
end_function_1871: ;
  return z8zE509;
end_block_exception_1872: ;

  return UNIT;
}

struct ztuple_z8z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256z9 zeff_gas_price_for(sail_u256 zbase_fee, sail_u256 zmax_fee, sail_u256 zmax_priority_fee)
{
  struct ztuple_z8z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256z9 z8zE510;
  sail_u256 zprice;
  {
    bool z2zE1466;
    z2zE1466 = zword_ule(zmax_fee, zbase_fee);
    if (z2zE1466) {  zprice = zmax_fee;  } else {
      sail_u256 zavailable_priority;
      zavailable_priority = zword_sub_word(zmax_fee, zbase_fee);
      bool z2zE1467;
      z2zE1467 = zword_ule(zmax_priority_fee, zavailable_priority);
      if (z2zE1467) {  zprice = zword_add_word(zbase_fee, zmax_priority_fee);  } else {  zprice = zmax_fee;  }
    }
  }
  sail_u256 zpriority;
  {
    bool z2zE1465;
    z2zE1465 = zword_ule(zbase_fee, zprice);
    if (z2zE1465) {  zpriority = zword_sub_word(zprice, zbase_fee);  } else {  zpriority = zZERO_WORD;  }
  }
  struct ztuple_z8z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256z9 z3zE1432;
  z3zE1432.ztup0 = zprice;
  z3zE1432.ztup1 = zpriority;
  z8zE510 = z3zE1432;
end_function_1857: ;
  return z8zE510;
end_block_exception_1858: ;
  struct ztuple_z8z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256z9 z8zE892 = { .ztup0 = u256_zero(), .ztup1 = u256_zero() };
  return z8zE892;
}

struct zTxValidity zcheck_transaction_validity(struct zTransaction ztx)
{
  struct zTxValidity z8zE511;
  bool z2zE1351;
  {
    bool z2zE1350;
    {
      enum zTxType z2zE1348;
      z2zE1348 = ztx.ztx_type;
      sail_u256 z2zE1349;
      z2zE1349 = ztx.zsig_v;
      z2zE1350 = ztx_sig_v_valid(zk_chain_id, z2zE1348, z2zE1349);
    }
    z2zE1351 = not(z2zE1350);
  }
  unit z3zE1385;
  if (z2zE1351) {
    struct zexception z2zE1352;
    CREATE(zexception)(&z2zE1352);
    zInvalidBlock(&z2zE1352, zInvalidSignature);
    COPY(zexception)(current_exception, z2zE1352);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/transaction.sail:558.8-558.44");
    KILL(zexception)(&z2zE1352);
    goto end_block_exception_1856;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1352);
  } else {  z3zE1385 = UNIT;  }
  uint64_t zparity;
  {
    enum zTxType z2zE1463;
    z2zE1463 = ztx.ztx_type;
    sail_u256 z2zE1464;
    z2zE1464 = ztx.zsig_v;
    zparity = ztx_y_parity(z2zE1463, z2zE1464);
  }
  bool z2zE1358;
  {
    bool z2zE1357;
    {
      sail_fixed_bytes_20 z2zE1353;
      z2zE1353 = ztx.zsender;
      sail_fixed_bytes_32 z2zE1354;
      z2zE1354 = ztx.zsigning_hash;
      sail_u256 z2zE1355;
      z2zE1355 = ztx.zsig_r;
      sail_u256 z2zE1356;
      z2zE1356 = ztx.zsig_s;
      z2zE1357 = ztx_auth_valid(z2zE1353, z2zE1354, zparity, z2zE1355, z2zE1356);
    }
    z2zE1358 = not(z2zE1357);
  }
  unit z3zE1386;
  if (z2zE1358) {
    struct zexception z2zE1359;
    CREATE(zexception)(&z2zE1359);
    zInvalidBlock(&z2zE1359, zInvalidSignature);
    COPY(zexception)(current_exception, z2zE1359);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/transaction.sail:562.8-562.44");
    KILL(zexception)(&z2zE1359);
    goto end_block_exception_1856;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1359);
  } else {  z3zE1386 = UNIT;  }
  uint64_t zgas_limit;
  {
    uint64_t z2zE1461;
    z2zE1461 = ztx.zgas_limit;
    uint64_t z2zE1462;
    z2zE1462 = zk_header.zgas_limit;
    {
      zgas_limit = zadmitted_transaction_gas_limit(z2zE1461, z2zE1462);
      if (have_exception) {  goto end_block_exception_1856;  }
    }
  }
  struct ztuple_z8z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256z9 z2zE1363;
  {
    sail_u256 z2zE1360;
    z2zE1360 = zk_header.zbase_fee;
    sail_u256 z2zE1361;
    z2zE1361 = ztx.zmax_fee;
    sail_u256 z2zE1362;
    z2zE1362 = ztx.zmax_priority_fee;
    z2zE1363 = zeff_gas_price_for(z2zE1360, z2zE1361, z2zE1362);
  }
  struct zTxValidity z3zE1387;
  {
    sail_u256 zeff_gas_price;
    zeff_gas_price = z2zE1363.ztup0;
    sail_u256 zeff_priority_fee;
    zeff_priority_fee = z2zE1363.ztup1;
    sail_fixed_bytes_20 zsender;
    zsender = ztx.zsender;
    struct zByteSliceFields zinput;
    zinput = ztx.zinput_src;
    uint64_t zinput_len;
    zinput_len = zinput.zlen;
    uint64_t znonce_before;
    {
      znonce_before = zk_get_nonce(zsender);
      if (have_exception) {  goto end_block_exception_1856;  }
    }
    sail_u256 zblob_price;
    {
      uint64_t z2zE1460;
      z2zE1460 = zk_header.zexcess_blob_gas;
      {
        zblob_price = zblob_base_fee(z2zE1460);
        if (have_exception) {  goto end_block_exception_1856;  }
      }
    }
    struct zTransactionCosts zcosts;
    {
      zcosts = ztransaction_costs(ztx, zgas_limit, zblob_price);
      if (have_exception) {  goto end_block_exception_1856;  }
    }
    uint64_t znonce;
    {
      struct zoptionzIU64zK z2zE1458;
      CREATE(zoptionzIU64zK)(&z2zE1458);
      {
        sail_u256 z2zE1457;
        z2zE1457 = ztx.znonce;
        zword_to_account_nonce(&z2zE1458, z2zE1457);
      }
      uint64_t z3zE1388;
      {
        if (z2zE1458.kind != Kind_zSomezIU64zK) goto case_1849;
        uint64_t z1zE23;
        z1zE23 = z2zE1458.variants.zSomezIU64zK;
        z3zE1388 = z1zE23;
        goto finish_match_1847;
      }
    case_1849: ;
      {
        /* complete */
        struct zexception z2zE1459;
        CREATE(zexception)(&z2zE1459);
        zInvalidBlock(&z2zE1459, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1459);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:576.18-576.54");
        KILL(zoptionzIU64zK)(&z2zE1458);
        KILL(zexception)(&z2zE1459);
        goto end_block_exception_1856;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1459);
        goto finish_match_1847;
      }
    case_1848: ;
    finish_match_1847: ;
      znonce = z3zE1388;
      KILL(zoptionzIU64zK)(&z2zE1458);
    }
    bool z2zE1364;
    z2zE1364 = (znonce != znonce_before);
    unit z3zE1391;
    if (z2zE1364) {
      struct zexception z2zE1365;
      CREATE(zexception)(&z2zE1365);
      zInvalidBlock(&z2zE1365, zExecutionInvalid);
      COPY(zexception)(current_exception, z2zE1365);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/evm/transaction.sail:579.8-579.44");
      KILL(zexception)(&z2zE1365);
      goto end_block_exception_1856;
      /* unreachable after throw */
      KILL(zexception)(&z2zE1365);
    } else {  z3zE1391 = UNIT;  }
    struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1366;
    {
      z2zE1366 = zk_deleg_target(zsender);
      if (have_exception) {  goto end_block_exception_1856;  }
    }
    struct zTxValidity z3zE1392;
    {
      bool zsender_deleg;
      zsender_deleg = z2zE1366.ztup0;
      uint64_t zmax_blobs;
      zmax_blobs = zmax_blobs_per_transaction(UNIT);
      bool z2zE1368;
      {
        enum zTxType z2zE1367;
        z2zE1367 = ztx.ztx_type;
        z2zE1368 = ztx_is_blob(z2zE1367);
      }
      unit z3zE1398;
      if (z2zE1368) {
        bool z2zE1376;
        {
          bool z2zE1375;
          z2zE1375 = zfork_lt(zk_fork, zCancun);
          bool z3zE1401;
          if (z2zE1375) {  z3zE1401 = true;  } else {
            bool z2zE1374;
            {
              uint64_t z2zE1370;
              {
                struct zBlobHashes z2zE1369;
                z2zE1369 = ztx.zblob_hashes;
                z2zE1370 = z2zE1369.zcount;
              }
              z2zE1374 = (z2zE1370 == UINT64_C(0));
            }
            bool z3zE1400;
            if (z2zE1374) {  z3zE1400 = true;  } else {
              bool z2zE1373;
              {
                uint64_t z2zE1372;
                {
                  struct zBlobHashes z2zE1371;
                  z2zE1371 = ztx.zblob_hashes;
                  z2zE1372 = z2zE1371.zcount;
                }
                z2zE1373 = (zmax_blobs < z2zE1372);
              }
              bool z3zE1399;
              if (z2zE1373) {  z3zE1399 = true;  } else {  z3zE1399 = ztx.zis_create;  }
              z3zE1400 = z3zE1399;
            }
            z3zE1401 = z3zE1400;
          }
          z2zE1376 = z3zE1401;
        }
        unit z3zE1402;
        if (z2zE1376) {
          struct zexception z2zE1377;
          CREATE(zexception)(&z2zE1377);
          zInvalidBlock(&z2zE1377, zExecutionInvalid);
          COPY(zexception)(current_exception, z2zE1377);
          have_exception = true;
          COPY(sail_string)(throw_location, "sail/evm/transaction.sail:593.12-593.48");
          KILL(zexception)(&z2zE1377);
          goto end_block_exception_1856;
          /* unreachable after throw */
          KILL(zexception)(&z2zE1377);
        } else {  z3zE1402 = UNIT;  }
        struct zBlobHashes z2zE1378;
        z2zE1378 = ztx.zblob_hashes;
        {
          z3zE1398 = zvalidate_blob_hash_versions(z2zE1378);
          if (have_exception) {  goto end_block_exception_1856;  }
        }
      } else {  z3zE1398 = UNIT;  }
      bool z2zE1381;
      {
        bool z2zE1380;
        z2zE1380 = zfork_gteq(zk_fork, zPrague);
        bool z3zE1393;
        if (z2zE1380) {
          uint64_t z2zE1379;
          z2zE1379 = zcosts.zcalldata_floor;
          z3zE1393 = (zgas_limit < z2zE1379);
        } else {  z3zE1393 = false;  }
        z2zE1381 = z3zE1393;
      }
      unit z3zE1397;
      if (z2zE1381) {
        struct zexception z2zE1382;
        CREATE(zexception)(&z2zE1382);
        zInvalidBlock(&z2zE1382, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1382);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:601.8-601.44");
        KILL(zexception)(&z2zE1382);
        goto end_block_exception_1856;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1382);
      } else {  z3zE1397 = UNIT;  }
      bool z2zE1386;
      {
        bool z2zE1385;
        {
          sail_u256 z2zE1383;
          z2zE1383 = zcosts.zupfront;
          sail_u256 z2zE1384;
          {
            z2zE1384 = zk_get_balance(zsender);
            if (have_exception) {  goto end_block_exception_1856;  }
          }
          z2zE1385 = zword_ule(z2zE1383, z2zE1384);
        }
        z2zE1386 = not(z2zE1385);
      }
      unit z3zE1396;
      if (z2zE1386) {
        struct zexception z2zE1387;
        CREATE(zexception)(&z2zE1387);
        zInvalidBlock(&z2zE1387, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1387);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:604.8-604.44");
        KILL(zexception)(&z2zE1387);
        goto end_block_exception_1856;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1387);
      } else {  z3zE1396 = UNIT;  }
      bool z2zE1391;
      {
        bool z2zE1390;
        {
          bool z2zE1389;
          {
            sail_fixed_bytes_32 z2zE1388;
            {
              z2zE1388 = zk_code_key(zsender);
              if (have_exception) {  goto end_block_exception_1856;  }
            }
            z2zE1389 = eq_fixed_bytes_32(z2zE1388, zKECCAK_EMPTY);
          }
          bool z3zE1394;
          if (z2zE1389) {  z3zE1394 = true;  } else {  z3zE1394 = zsender_deleg;  }
          z2zE1390 = z3zE1394;
        }
        z2zE1391 = not(z2zE1390);
      }
      unit z3zE1395;
      if (z2zE1391) {
        struct zexception z2zE1392;
        CREATE(zexception)(&z2zE1392);
        zInvalidBlock(&z2zE1392, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1392);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:607.8-607.44");
        KILL(zexception)(&z2zE1392);
        goto end_block_exception_1856;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1392);
      } else {  z3zE1395 = UNIT;  }
      sail_u128 zintrinsic_total;
      {
        uint64_t z2zE1455;
        z2zE1455 = zcosts.zintrinsic_execution;
        uint64_t z2zE1456;
        z2zE1456 = zcosts.zintrinsic_state;
        zintrinsic_total = u128_add_u64_u64(z2zE1455, z2zE1456);
      }
      bool z2zE1393;
      z2zE1393 = u64_lt_u128(zgas_limit, zintrinsic_total);
      unit z3zE1428;
      if (z2zE1393) {
        struct zexception z2zE1394;
        CREATE(zexception)(&z2zE1394);
        zInvalidBlock(&z2zE1394, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1394);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:611.8-611.44");
        KILL(zexception)(&z2zE1394);
        goto end_block_exception_1856;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1394);
      } else {  z3zE1428 = UNIT;  }
      bool z2zE1397;
      {
        bool z2zE1396;
        z2zE1396 = zfork_gteq(zk_fork, zAmsterdam);
        bool z3zE1403;
        if (z2zE1396) {
          uint64_t z2zE1395;
          z2zE1395 = zcosts.zintrinsic_execution;
          z3zE1403 = (zAMSTERDAM_TX_MAX_GAS < z2zE1395);
        } else {  z3zE1403 = false;  }
        z2zE1397 = z3zE1403;
      }
      unit z3zE1427;
      if (z2zE1397) {
        struct zexception z2zE1398;
        CREATE(zexception)(&z2zE1398);
        zInvalidBlock(&z2zE1398, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1398);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:614.8-614.44");
        KILL(zexception)(&z2zE1398);
        goto end_block_exception_1856;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1398);
      } else {  z3zE1427 = UNIT;  }
      bool z2zE1401;
      {
        bool z2zE1400;
        z2zE1400 = zfork_gteq(zk_fork, zAmsterdam);
        bool z3zE1404;
        if (z2zE1400) {
          uint64_t z2zE1399;
          z2zE1399 = zcosts.zcalldata_floor;
          z3zE1404 = (zAMSTERDAM_TX_MAX_GAS < z2zE1399);
        } else {  z3zE1404 = false;  }
        z2zE1401 = z3zE1404;
      }
      unit z3zE1426;
      if (z2zE1401) {
        struct zexception z2zE1402;
        CREATE(zexception)(&z2zE1402);
        zInvalidBlock(&z2zE1402, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1402);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:617.8-617.44");
        KILL(zexception)(&z2zE1402);
        goto end_block_exception_1856;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1402);
      } else {  z3zE1426 = UNIT;  }
      bool z2zE1406;
      {
        bool z2zE1405;
        {
          sail_u256 z2zE1403;
          z2zE1403 = zk_header.zbase_fee;
          sail_u256 z2zE1404;
          z2zE1404 = ztx.zmax_fee;
          z2zE1405 = zword_ule(z2zE1403, z2zE1404);
        }
        z2zE1406 = not(z2zE1405);
      }
      unit z3zE1425;
      if (z2zE1406) {
        struct zexception z2zE1407;
        CREATE(zexception)(&z2zE1407);
        zInvalidBlock(&z2zE1407, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1407);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:620.8-620.44");
        KILL(zexception)(&z2zE1407);
        goto end_block_exception_1856;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1407);
      } else {  z3zE1425 = UNIT;  }
      bool z2zE1413;
      {
        bool z2zE1412;
        {
          uint64_t z2zE1409;
          {
            struct zBlobHashes z2zE1408;
            z2zE1408 = ztx.zblob_hashes;
            z2zE1409 = z2zE1408.zcount;
          }
          z2zE1412 = (z2zE1409 != UINT64_C(0));
        }
        bool z3zE1405;
        if (z2zE1412) {
          bool z2zE1411;
          {
            sail_u256 z2zE1410;
            z2zE1410 = ztx.zmax_blob_fee;
            z2zE1411 = zword_ule(zblob_price, z2zE1410);
          }
          z3zE1405 = not(z2zE1411);
        } else {  z3zE1405 = false;  }
        z2zE1413 = z3zE1405;
      }
      unit z3zE1424;
      if (z2zE1413) {
        struct zexception z2zE1414;
        CREATE(zexception)(&z2zE1414);
        zInvalidBlock(&z2zE1414, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1414);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:623.8-623.44");
        KILL(zexception)(&z2zE1414);
        goto end_block_exception_1856;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1414);
      } else {  z3zE1424 = UNIT;  }
      bool z2zE1417;
      {
        bool z2zE1416;
        z2zE1416 = ztx.zis_create;
        bool z3zE1406;
        if (z2zE1416) {
          bool z2zE1415;
          z2zE1415 = zinitcode_sizze_allowedzIreprzGU64zCozKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zinput_len);
          z3zE1406 = not(z2zE1415);
        } else {  z3zE1406 = false;  }
        z2zE1417 = z3zE1406;
      }
      unit z3zE1423;
      if (z2zE1417) {
        struct zexception z2zE1418;
        CREATE(zexception)(&z2zE1418);
        zInvalidBlock(&z2zE1418, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1418);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:626.8-626.44");
        KILL(zexception)(&z2zE1418);
        goto end_block_exception_1856;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1418);
      } else {  z3zE1423 = UNIT;  }
      bool z2zE1422;
      {
        bool z2zE1421;
        {
          sail_u256 z2zE1419;
          z2zE1419 = ztx.zmax_priority_fee;
          sail_u256 z2zE1420;
          z2zE1420 = ztx.zmax_fee;
          z2zE1421 = zword_ule(z2zE1419, z2zE1420);
        }
        z2zE1422 = not(z2zE1421);
      }
      unit z3zE1422;
      if (z2zE1422) {
        struct zexception z2zE1423;
        CREATE(zexception)(&z2zE1423);
        zInvalidBlock(&z2zE1423, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1423);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:629.8-629.44");
        KILL(zexception)(&z2zE1423);
        goto end_block_exception_1856;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1423);
      } else {  z3zE1422 = UNIT;  }
      bool z2zE1426;
      {
        bool z2zE1425;
        {
          enum zTxType z2zE1424;
          z2zE1424 = ztx.ztx_type;
          z2zE1425 = ztx_is_access_list(z2zE1424);
        }
        bool z3zE1407;
        if (z2zE1425) {  z3zE1407 = zfork_lt(zk_fork, zBerlin);  } else {  z3zE1407 = false;  }
        z2zE1426 = z3zE1407;
      }
      unit z3zE1421;
      if (z2zE1426) {
        struct zexception z2zE1427;
        CREATE(zexception)(&z2zE1427);
        zInvalidBlock(&z2zE1427, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1427);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:632.8-632.44");
        KILL(zexception)(&z2zE1427);
        goto end_block_exception_1856;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1427);
      } else {  z3zE1421 = UNIT;  }
      bool z2zE1430;
      {
        bool z2zE1429;
        {
          enum zTxType z2zE1428;
          z2zE1428 = ztx.ztx_type;
          z2zE1429 = ztx_is_dynamic_fee(z2zE1428);
        }
        bool z3zE1408;
        if (z2zE1429) {  z3zE1408 = zfork_lt(zk_fork, zLondon);  } else {  z3zE1408 = false;  }
        z2zE1430 = z3zE1408;
      }
      unit z3zE1420;
      if (z2zE1430) {
        struct zexception z2zE1431;
        CREATE(zexception)(&z2zE1431);
        zInvalidBlock(&z2zE1431, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1431);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:635.8-635.44");
        KILL(zexception)(&z2zE1431);
        goto end_block_exception_1856;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1431);
      } else {  z3zE1420 = UNIT;  }
      bool z2zE1434;
      {
        bool z2zE1433;
        {
          enum zTxType z2zE1432;
          z2zE1432 = ztx.ztx_type;
          z2zE1433 = ztx_is_set_code(z2zE1432);
        }
        bool z3zE1409;
        if (z2zE1433) {  z3zE1409 = ztx.zis_create;  } else {  z3zE1409 = false;  }
        z2zE1434 = z3zE1409;
      }
      unit z3zE1419;
      if (z2zE1434) {
        struct zexception z2zE1435;
        CREATE(zexception)(&z2zE1435);
        zInvalidBlock(&z2zE1435, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1435);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:638.8-638.44");
        KILL(zexception)(&z2zE1435);
        goto end_block_exception_1856;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1435);
      } else {  z3zE1419 = UNIT;  }
      bool z2zE1439;
      {
        bool z2zE1438;
        {
          enum zTxType z2zE1436;
          z2zE1436 = ztx.ztx_type;
          z2zE1438 = ztx_is_set_code(z2zE1436);
        }
        bool z3zE1410;
        if (z2zE1438) {
          uint64_t z2zE1437;
          z2zE1437 = ztx.zauthorizzation_count;
          z3zE1410 = (z2zE1437 == UINT64_C(0));
        } else {  z3zE1410 = false;  }
        z2zE1439 = z3zE1410;
      }
      unit z3zE1418;
      if (z2zE1439) {
        struct zexception z2zE1440;
        CREATE(zexception)(&z2zE1440);
        zInvalidBlock(&z2zE1440, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1440);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:641.8-641.44");
        KILL(zexception)(&z2zE1440);
        goto end_block_exception_1856;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1440);
      } else {  z3zE1418 = UNIT;  }
      bool z2zE1443;
      {
        bool z2zE1442;
        {
          enum zTxType z2zE1441;
          z2zE1441 = ztx.ztx_type;
          z2zE1442 = ztx_is_set_code(z2zE1441);
        }
        bool z3zE1411;
        if (z2zE1442) {  z3zE1411 = zfork_lt(zk_fork, zPrague);  } else {  z3zE1411 = false;  }
        z2zE1443 = z3zE1411;
      }
      unit z3zE1417;
      if (z2zE1443) {
        struct zexception z2zE1444;
        CREATE(zexception)(&z2zE1444);
        zInvalidBlock(&z2zE1444, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1444);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:644.8-644.44");
        KILL(zexception)(&z2zE1444);
        goto end_block_exception_1856;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1444);
      } else {  z3zE1417 = UNIT;  }
      bool z2zE1447;
      {
        enum zTxType z2zE1445;
        z2zE1445 = ztx.ztx_type;
        bool z3zE1412;
        {
          if ((zLegacyTx != z2zE1445)) goto case_1854;
          z3zE1412 = false;
          goto finish_match_1852;
        }
      case_1854: ;
        {
          uint64_t z2zE1446;
          z2zE1446 = ztx.zchain_id;
          z3zE1412 = (z2zE1446 != zk_chain_id);
          goto finish_match_1852;
        }
      case_1853: ;
      finish_match_1852: ;
        z2zE1447 = z3zE1412;
      }
      unit z3zE1416;
      if (z2zE1447) {
        struct zexception z2zE1448;
        CREATE(zexception)(&z2zE1448);
        zInvalidBlock(&z2zE1448, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1448);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:650.8-650.44");
        KILL(zexception)(&z2zE1448);
        goto end_block_exception_1856;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1448);
      } else {  z3zE1416 = UNIT;  }
      bool z2zE1449;
      z2zE1449 = (znonce_before == UINT64_C(18446744073709551615));
      unit z3zE1415;
      if (z2zE1449) {
        struct zexception z2zE1450;
        CREATE(zexception)(&z2zE1450);
        zInvalidBlock(&z2zE1450, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1450);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:653.8-653.44");
        KILL(zexception)(&z2zE1450);
        goto end_block_exception_1856;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1450);
      } else {  z3zE1415 = UNIT;  }
      uint64_t z2zE1451;
      z2zE1451 = zcosts.zintrinsic_execution;
      uint64_t z2zE1452;
      z2zE1452 = zcosts.zintrinsic_state;
      uint64_t z2zE1453;
      z2zE1453 = zcosts.zcalldata_floor;
      sail_u256 z2zE1454;
      z2zE1454 = zcosts.zblob_fee;
      struct zTxValidity z3zE1429;
      z3zE1429.zblob_fee = z2zE1454;
      z3zE1429.zcalldata_floor = z2zE1453;
      z3zE1429.zgas_limit = zgas_limit;
      z3zE1429.zgas_price = zeff_gas_price;
      z3zE1429.zintrinsic_execution_gas = z2zE1451;
      z3zE1429.zintrinsic_state_gas = z2zE1452;
      z3zE1429.znonce_before = znonce_before;
      z3zE1429.zpriority_fee = zeff_priority_fee;
      z3zE1429.zsender = zsender;
      z3zE1392 = z3zE1429;
      goto finish_match_1850;
    }
  case_1851: ;
    sail_match_failure("check_transaction_validity");
  finish_match_1850: ;
    z3zE1387 = z3zE1392;
    goto finish_match_1845;
  }
case_1846: ;
  sail_match_failure("check_transaction_validity");
finish_match_1845: ;
  z8zE511 = z3zE1387;
end_function_1855: ;
  return z8zE511;
end_block_exception_1856: ;
  struct zTxValidity z8zE893 = { .zblob_fee = u256_zero(), .zcalldata_floor = UINT64_C(0xdeadc0de), .zgas_limit = UINT64_C(0xdeadc0de), .zgas_price = u256_zero(), .zintrinsic_execution_gas = UINT64_C(0xdeadc0de), .zintrinsic_state_gas = UINT64_C(0xdeadc0de), .znonce_before = UINT64_C(0xdeadc0de), .zpriority_fee = u256_zero(), .zsender = fixed_bytes_20_zero() };
  return z8zE893;
}

struct zTxUpfrontResult zapply_transaction_upfront_effects(struct zTransaction ztx, struct zTxValidity zv)
{
  struct zTxUpfrontResult z8zE512;
  bool zcreate_target_prestate_empty;
  {
    bool z2zE1343;
    {
      bool z2zE1342;
      z2zE1342 = zfork_gteq(zk_fork, zAmsterdam);
      bool z3zE1379;
      if (z2zE1342) {  z3zE1379 = ztx.zis_create;  } else {  z3zE1379 = false;  }
      z2zE1343 = z3zE1379;
    }
    if (z2zE1343) {
      bool z2zE1347;
      {
        sail_fixed_bytes_20 z2zE1346;
        {
          sail_fixed_bytes_20 z2zE1344;
          z2zE1344 = zv.zsender;
          uint64_t z2zE1345;
          z2zE1345 = zv.znonce_before;
          z2zE1346 = zk_create_addr(z2zE1344, z2zE1345);
        }
        {
          z2zE1347 = zk_account_exists(z2zE1346);
          if (have_exception) {  goto end_block_exception_1844;  }
        }
      }
      zcreate_target_prestate_empty = not(z2zE1347);
    } else {  zcreate_target_prestate_empty = false;  }
  }
  sail_fixed_bytes_20 z2zE1333;
  z2zE1333 = zv.zsender;
  sail_u256 z2zE1334;
  {
    sail_u256 z2zE1331;
    z2zE1331 = zv.zgas_price;
    uint64_t z2zE1332;
    z2zE1332 = zv.zgas_limit;
    {
      z2zE1334 = zvalidated_word_productzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE1331, z2zE1332);
      if (have_exception) {  goto end_block_exception_1844;  }
    }
  }
  unit z3zE1383;
  {
    z3zE1383 = zk_sub_balance(z2zE1333, z2zE1334);
    if (have_exception) {  goto end_block_exception_1844;  }
  }
  bool z2zE1336;
  {
    sail_u256 z2zE1335;
    z2zE1335 = zv.zblob_fee;
    z2zE1336 = zword_nonzzero(z2zE1335);
  }
  unit z3zE1382;
  if (z2zE1336) {
    sail_fixed_bytes_20 z2zE1337;
    z2zE1337 = zv.zsender;
    sail_u256 z2zE1338;
    z2zE1338 = zv.zblob_fee;
    {
      z3zE1382 = zk_sub_balance(z2zE1337, z2zE1338);
      if (have_exception) {  goto end_block_exception_1844;  }
    }
  } else {  z3zE1382 = UNIT;  }
  sail_fixed_bytes_20 z2zE1339;
  z2zE1339 = zv.zsender;
  unit z3zE1381;
  {
    z3zE1381 = zk_bump_nonce(z2zE1339);
    if (have_exception) {  goto end_block_exception_1844;  }
  }
  unit z3zE1380;
  z3zE1380 = zprewarm(ztx);
  __int128 zauthorizzation_refund;
  {
    bool z2zE1340;
    z2zE1340 = zfork_lt(zk_fork, zAmsterdam);
    if (z2zE1340) {
      zz5listz8z5structz0zzAuthorizzzzationz9 z2zE1341;
      CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE1341);
      COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE1341, ztx.zauthorizzations);
      {
        zauthorizzation_refund = zprocess_auth_list(z2zE1341);
        if (have_exception) {
          KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE1341);
          goto end_block_exception_1844;
        }
      }
      KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE1341);
    } else {  zauthorizzation_refund = zGAS_REFUND_ZERO;  }
  }
  struct zTxUpfrontResult z3zE1384;
  z3zE1384.zauthorizzation_refund = zauthorizzation_refund;
  z3zE1384.zcreate_target_prestate_empty = zcreate_target_prestate_empty;
  z8zE512 = z3zE1384;
end_function_1843: ;
  return z8zE512;
end_block_exception_1844: ;
  struct zTxUpfrontResult z8zE894 = { .zauthorizzation_refund = ((__int128)INT64_C(0xdeadc0de)), .zcreate_target_prestate_empty = false };
  return z8zE894;
}

unit zenter_transaction_frame(uint64_t zgas_limit, uint64_t zintrinsic_execution, uint64_t zintrinsic_state)
{
  unit z8zE513;
  zpc = UINT64_C(0);
  unit z3zE1365;
  z3zE1365 = UNIT;
  zcall_depth = UINT64_C(0);
  unit z3zE1364;
  z3zE1364 = UNIT;
  uint64_t zafter_execution_intrinsic;
  {
    zafter_execution_intrinsic = zvalidated_gas_subzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zgas_limit, zintrinsic_execution);
    if (have_exception) {  goto end_block_exception_1842;  }
  }
  uint64_t zavailable_gas;
  {
    zavailable_gas = zvalidated_gas_subzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zafter_execution_intrinsic, zintrinsic_state);
    if (have_exception) {  goto end_block_exception_1842;  }
  }
  bool z2zE1329;
  z2zE1329 = zfork_gteq(zk_fork, zAmsterdam);
  unit z3zE1375;
  if (z2zE1329) {
    uint64_t zexecution_budget;
    {
      zexecution_budget = zvalidated_gas_subzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zAMSTERDAM_TX_MAX_GAS, zintrinsic_execution);
      if (have_exception) {  goto end_block_exception_1842;  }
    }
    bool z2zE1330;
    z2zE1330 = (zavailable_gas < zexecution_budget);
    if (z2zE1330) {
      zgas_remaining = zavailable_gas;
      unit z3zE1378;
      z3zE1378 = UNIT;
      zstate_gas_remaining = zGAS_ZERO;
      z3zE1375 = UNIT;
    } else {
      zgas_remaining = zexecution_budget;
      unit z3zE1377;
      z3zE1377 = UNIT;
      uint64_t zremaining_state_gas;
      {
        zremaining_state_gas = zvalidated_gas_subzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zavailable_gas, zexecution_budget);
        if (have_exception) {  goto end_block_exception_1842;  }
      }
      zstate_gas_remaining = zremaining_state_gas;
      z3zE1375 = UNIT;
    }
  } else {
    zgas_remaining = zavailable_gas;
    unit z3zE1376;
    z3zE1376 = UNIT;
    zstate_gas_remaining = zGAS_ZERO;
    z3zE1375 = UNIT;
  }
  zstate_gas_spilled = zSTATE_GAS_SPILL_ZERO;
  unit z3zE1374;
  z3zE1374 = UNIT;
  struct zMessage z3zE1366;
  z3zE1366.zaddress = zZERO_ADDRESS;
  z3zE1366.zcaller = zZERO_ADDRESS;
  z3zE1366.zcode_address = zZERO_ADDRESS;
  z3zE1366.zdepth = UINT64_C(0);
  z3zE1366.zis_static = false;
  z3zE1366.zstate_gas_reservoir = zstate_gas_remaining;
  z3zE1366.zvalue = zZERO_WORD;
  zmessage = z3zE1366;
  unit z3zE1373;
  z3zE1373 = UNIT;
  unit z3zE1372;
  z3zE1372 = stack_reset(UNIT);
  unit z3zE1371;
  z3zE1371 = zmemory_reset(UNIT);
  unit z3zE1370;
  z3zE1370 = zreturndata_clear(UNIT);
  zcalldata = zEMPTY_SLICE;
  unit z3zE1369;
  z3zE1369 = UNIT;
  zframe_code = zEMPTY_CODE;
  unit z3zE1368;
  z3zE1368 = UNIT;
  zframe_refund = zGAS_REFUND_ZERO;
  unit z3zE1367;
  z3zE1367 = UNIT;
  zRunning(&zframe_status, UNIT);
  z8zE513 = UNIT;
end_function_1841: ;
  return z8zE513;
end_block_exception_1842: ;

  return UNIT;
}

bool zprepare_amsterdam_transaction_dispatch(struct zTransaction ztx, struct zTxValidity zv, struct zTxUpfrontResult zupfront)
{
  bool z8zE514;
  sail_fixed_bytes_20 zcurrent_target;
  {
    bool z2zE1326;
    z2zE1326 = ztx.zis_create;
    if (z2zE1326) {
      sail_fixed_bytes_20 z2zE1327;
      z2zE1327 = zv.zsender;
      uint64_t z2zE1328;
      z2zE1328 = zv.znonce_before;
      zcurrent_target = zk_create_addr(z2zE1327, z2zE1328);
    } else {  zcurrent_target = ztx.zrecipient;  }
  }
  sail_fixed_bytes_20 z2zE1308;
  z2zE1308 = zv.zsender;
  sail_u256 z2zE1309;
  z2zE1309 = ztx.zvalue;
  struct zMessage z3zE1350;
  z3zE1350.zaddress = zcurrent_target;
  z3zE1350.zcaller = z2zE1308;
  z3zE1350.zcode_address = zcurrent_target;
  z3zE1350.zdepth = UINT64_C(0);
  z3zE1350.zis_static = false;
  z3zE1350.zstate_gas_reservoir = zstate_gas_remaining;
  z3zE1350.zvalue = z2zE1309;
  zmessage = z3zE1350;
  unit z3zE1351;
  z3zE1351 = UNIT;
  bool z2zE1310;
  z2zE1310 = ztx.zis_create;
  if (z2zE1310) {
    bool z2zE1311;
    z2zE1311 = zupfront.zcreate_target_prestate_empty;
    unit z3zE1363;
    if (z2zE1311) {
      {
        z3zE1363 = zcharge_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_amsterdam_state_new_account);
        if (have_exception) {  goto end_block_exception_1840;  }
      }
    } else {  z3zE1363 = UNIT;  }
    bool z2zE1312;
    z2zE1312 = zis_running(UNIT);
    unit z3zE1362;
    if (z2zE1312) {
      sail_fixed_bytes_32 z2zE1315;
      {
        struct zByteSliceFields z2zE1314;
        {
          struct zByteSliceFields z2zE1313;
          z2zE1313 = ztx.zinput_src;
          z2zE1314 = ztransaction_initcode_slice(z2zE1313);
        }
        z2zE1315 = zcode_db_insert(z2zE1314, zk_fork);
      }
      {
        zframe_code = zcode_db_resolve(z2zE1315);
        if (have_exception) {  goto end_block_exception_1840;  }
      }
      z3zE1362 = UNIT;
    } else {  z3zE1362 = UNIT;  }
    z8zE514 = false;
  } else {
    zcalldata = ztx.zinput_src;
    unit z3zE1354;
    z3zE1354 = UNIT;
    bool z2zE1319;
    {
      bool z2zE1318;
      {
        sail_u256 z2zE1316;
        z2zE1316 = ztx.zvalue;
        z2zE1318 = zword_nonzzero(z2zE1316);
      }
      bool z3zE1352;
      if (z2zE1318) {
        sail_fixed_bytes_20 z2zE1317;
        z2zE1317 = ztx.zrecipient;
        {
          z3zE1352 = zk_account_is_empty(z2zE1317);
          if (have_exception) {  goto end_block_exception_1840;  }
        }
      } else {  z3zE1352 = false;  }
      z2zE1319 = z3zE1352;
    }
    unit z3zE1353;
    if (z2zE1319) {
      {
        z3zE1353 = zcharge_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_amsterdam_state_new_account);
        if (have_exception) {  goto end_block_exception_1840;  }
      }
    } else {  z3zE1353 = UNIT;  }
    bool zdelegated;
    zdelegated = false;
    sail_fixed_bytes_20 zdelegate;
    zdelegate = zZERO_ADDRESS;
    bool z2zE1320;
    z2zE1320 = zis_running(UNIT);
    unit z3zE1357;
    if (z2zE1320) {
      struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1322;
      {
        sail_fixed_bytes_20 z2zE1321;
        z2zE1321 = ztx.zrecipient;
        {
          z2zE1322 = zk_deleg_target(z2zE1321);
          if (have_exception) {  goto end_block_exception_1840;  }
        }
      }
      unit z3zE1358;
      {
        bool zis_delegated;
        zis_delegated = z2zE1322.ztup0;
        sail_fixed_bytes_20 ztarget;
        ztarget = z2zE1322.ztup1;
        zdelegated = zis_delegated;
        unit z3zE1360;
        z3zE1360 = UNIT;
        zdelegate = ztarget;
        unit z3zE1359;
        z3zE1359 = UNIT;
        if (zdelegated) {
          bool zwarm;
          zwarm = zk_access_account(zdelegate);
          uint64_t z2zE1323;
          z2zE1323 = zaccount_cost(zwarm);
          {
            z3zE1358 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE1323);
            if (have_exception) {  goto end_block_exception_1840;  }
          }
        } else {  z3zE1358 = UNIT;  }
        goto finish_match_1837;
      }
    case_1838: ;
      sail_match_failure("prepare_amsterdam_transaction_dispatch");
    finish_match_1837: ;
      z3zE1357 = z3zE1358;
    } else {  z3zE1357 = UNIT;  }
    bool z2zE1324;
    z2zE1324 = zis_running(UNIT);
    unit z3zE1355;
    if (z2zE1324) {
      unit z3zE1356;
      if (zdelegated) {
        zmessage.zcode_address = zdelegate;
        z3zE1356 = UNIT;
      } else {  z3zE1356 = UNIT;  }
      sail_fixed_bytes_20 z2zE1325;
      z2zE1325 = ztx.zrecipient;
      {
        zframe_code = zexecutable_code(z2zE1325, zdelegated, zdelegate);
        if (have_exception) {  goto end_block_exception_1840;  }
      }
      z3zE1355 = UNIT;
    } else {  z3zE1355 = UNIT;  }
    z8zE514 = zdelegated;
  }
end_function_1839: ;
  return z8zE514;
end_block_exception_1840: ;

  return false;
}

unit zrun_create_transaction_frame(struct zTransaction ztx, sail_fixed_bytes_20 zsender, uint64_t znonce_before)
{
  unit z8zE515;
  sail_fixed_bytes_20 znew_addr;
  znew_addr = zk_create_addr(zsender, znonce_before);
  bool z2zE1287;
  z2zE1287 = zk_access_account(znew_addr);
  unit z3zE1332;
  {
    bool z2zE1288;
    {
      z2zE1288 = zk_account_occupied(znew_addr);
      if (have_exception) {  goto end_block_exception_1836;  }
    }
    if (z2zE1288) {
      {
        z3zE1332 = zexc_halt(zAddressCollision);
        if (have_exception) {  goto end_block_exception_1836;  }
      }
    } else {
      unit z3zE1340;
      {
        z3zE1340 = zk_mark_created(znew_addr);
        if (have_exception) {  goto end_block_exception_1836;  }
      }
      unit z3zE1339;
      {
        z3zE1339 = zk_clear_storage(znew_addr);
        if (have_exception) {  goto end_block_exception_1836;  }
      }
      unit z3zE1338;
      {
        z3zE1338 = zk_bump_nonce(znew_addr);
        if (have_exception) {  goto end_block_exception_1836;  }
      }
      bool z2zE1290;
      {
        sail_u256 z2zE1289;
        z2zE1289 = ztx.zvalue;
        z2zE1290 = zword_nonzzero(z2zE1289);
      }
      unit z3zE1337;
      if (z2zE1290) {
        sail_u256 z2zE1291;
        z2zE1291 = ztx.zvalue;
        {
          z3zE1337 = zk_transfer(zsender, znew_addr, z2zE1291);
          if (have_exception) {  goto end_block_exception_1836;  }
        }
      } else {  z3zE1337 = UNIT;  }
      bool z2zE1292;
      z2zE1292 = zfork_lt(zk_fork, zAmsterdam);
      unit z3zE1334;
      if (z2zE1292) {
        sail_u256 z2zE1293;
        z2zE1293 = ztx.zvalue;
        struct zMessage z3zE1335;
        z3zE1335.zaddress = znew_addr;
        z3zE1335.zcaller = zsender;
        z3zE1335.zcode_address = znew_addr;
        z3zE1335.zdepth = UINT64_C(0);
        z3zE1335.zis_static = false;
        z3zE1335.zstate_gas_reservoir = zstate_gas_remaining;
        z3zE1335.zvalue = z2zE1293;
        zmessage = z3zE1335;
        unit z3zE1336;
        z3zE1336 = UNIT;
        sail_fixed_bytes_32 z2zE1296;
        {
          struct zByteSliceFields z2zE1295;
          {
            struct zByteSliceFields z2zE1294;
            z2zE1294 = ztx.zinput_src;
            z2zE1295 = ztransaction_initcode_slice(z2zE1294);
          }
          z2zE1296 = zcode_db_insert(z2zE1295, zk_fork);
        }
        {
          zframe_code = zcode_db_resolve(z2zE1296);
          if (have_exception) {  goto end_block_exception_1836;  }
        }
        z3zE1334 = UNIT;
      } else {  z3zE1334 = UNIT;  }
      struct zByteSliceFields zdeployed_code;
      zdeployed_code = zinterpret(UNIT);
      bool z2zE1297;
      z2zE1297 = zframe_succeeded(UNIT);
      if (z2zE1297) {
        uint64_t zdep_len;
        zdep_len = zdeployed_code.zlen;
        uint64_t zdeployed_length;
        zdeployed_length = zdep_len;
        bool z2zE1302;
        {
          bool z2zE1301;
          z2zE1301 = zdeployed_code_sizze_allowedzIreprzGU64zCozKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zdeployed_length);
          bool z3zE1343;
          if (z2zE1301) {
            bool z2zE1300;
            z2zE1300 = zfork_lt(zk_fork, zLondon);
            bool z3zE1342;
            if (z2zE1300) {  z3zE1342 = true;  } else {
              bool z2zE1299;
              z2zE1299 = (zdeployed_length == UINT64_C(0));
              bool z3zE1341;
              if (z2zE1299) {  z3zE1341 = true;  } else {
                uint64_t z2zE1298;
                z2zE1298 = zslice_byte(zdeployed_code, UINT64_C(0));
                z3zE1341 = (z2zE1298 != UINT64_C(0xEF));
              }
              z3zE1342 = z3zE1341;
            }
            z3zE1343 = z3zE1342;
          } else {  z3zE1343 = false;  }
          z2zE1302 = z3zE1343;
        }
        if (z2zE1302) {
          struct zoptionzIU64zK z2zE1303;
          CREATE(zoptionzIU64zK)(&z2zE1303);
          zcode_deployment_execution_cost(&z2zE1303, zdep_len, zgas_remaining);
          unit z3zE1344;
          {
            if (z2zE1303.kind != Kind_zSomezIU64zK) goto case_1834;
            uint64_t zexecution_deposit;
            zexecution_deposit = z2zE1303.variants.zSomezIU64zK;
            {
              zgas_remaining = zgas_sub_or_oog(zgas_remaining, zexecution_deposit);
              if (have_exception) {
                KILL(zoptionzIU64zK)(&z2zE1303);
                goto end_block_exception_1836;
              }
            }
            unit z3zE1348;
            z3zE1348 = UNIT;
            uint64_t z2zE1304;
            {
              z2zE1304 = zcode_deployment_state_cost(zdep_len);
              if (have_exception) {
                KILL(zoptionzIU64zK)(&z2zE1303);
                goto end_block_exception_1836;
              }
            }
            unit z3zE1347;
            {
              z3zE1347 = zcharge_deployment_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE1304);
              if (have_exception) {
                KILL(zoptionzIU64zK)(&z2zE1303);
                goto end_block_exception_1836;
              }
            }
            bool z2zE1305;
            z2zE1305 = zframe_succeeded(UNIT);
            if (z2zE1305) {
              struct zByteSliceFields z2zE1306;
              z2zE1306 = zvalidated_code_slice(zdeployed_code);
              {
                z3zE1344 = zk_deploy_code(znew_addr, z2zE1306);
                if (have_exception) {
                  KILL(zoptionzIU64zK)(&z2zE1303);
                  goto end_block_exception_1836;
                }
              }
            } else {  z3zE1344 = UNIT;  }
            goto finish_match_1832;
          }
        case_1834: ;
          {
            /* complete */
            bool z2zE1307;
            z2zE1307 = zfork_lt(zk_fork, zHomestead);
            if (z2zE1307) {
              zgas_remaining = zGAS_ZERO;
              unit z3zE1346;
              z3zE1346 = UNIT;
              {
                z3zE1344 = zk_deploy_code(znew_addr, zEMPTY_CODE_SLICE);
                if (have_exception) {
                  KILL(zoptionzIU64zK)(&z2zE1303);
                  goto end_block_exception_1836;
                }
              }
            } else {
              {
                z3zE1344 = zexc_halt(zOutOfGas);
                if (have_exception) {
                  KILL(zoptionzIU64zK)(&z2zE1303);
                  goto end_block_exception_1836;
                }
              }
            }
            goto finish_match_1832;
          }
        case_1833: ;
        finish_match_1832: ;
          z3zE1332 = z3zE1344;
          KILL(zoptionzIU64zK)(&z2zE1303);
        } else {
          {
            z3zE1332 = zexc_halt(zOutOfGas);
            if (have_exception) {  goto end_block_exception_1836;  }
          }
        }
      } else {  z3zE1332 = UNIT;  }
    }
    goto finish_match_1830;
  }
case_1831: ;
  sail_match_failure("run_create_transaction_frame");
finish_match_1830: ;
  z8zE515 = z3zE1332;
end_function_1835: ;
  return z8zE515;
end_block_exception_1836: ;

  return UNIT;
}

unit zrun_call_transaction_frame(struct zTransaction ztx, sail_fixed_bytes_20 zsender, bool zdelegated)
{
  unit z8zE516;
  struct zAccount z2zE1262;
  {
    sail_fixed_bytes_20 z2zE1261;
    z2zE1261 = ztx.zrecipient;
    {
      z2zE1262 = zk_aload(z2zE1261);
      if (have_exception) {  goto end_block_exception_1829;  }
    }
  }
  unit z3zE1311;
  {
    bool z2zE1264;
    {
      sail_u256 z2zE1263;
      z2zE1263 = ztx.zvalue;
      z2zE1264 = zword_nonzzero(z2zE1263);
    }
    unit z3zE1312;
    if (z2zE1264) {
      sail_fixed_bytes_20 z2zE1265;
      z2zE1265 = ztx.zrecipient;
      sail_u256 z2zE1266;
      z2zE1266 = ztx.zvalue;
      {
        z3zE1312 = zk_transfer(zsender, z2zE1265, z2zE1266);
        if (have_exception) {  goto end_block_exception_1829;  }
      }
    } else {  z3zE1312 = UNIT;  }
    uint64_t zselected_precompile;
    {
      sail_fixed_bytes_20 z2zE1286;
      z2zE1286 = ztx.zrecipient;
      zselected_precompile = zprecompile_number(z2zE1286);
    }
    bool z2zE1268;
    {
      bool z2zE1267;
      z2zE1267 = not(zdelegated);
      bool z3zE1313;
      if (z2zE1267) {  z3zE1313 = (zselected_precompile != UINT64_C(0));  } else {  z3zE1313 = false;  }
      z2zE1268 = z3zE1313;
    }
    if (z2zE1268) {
      uint64_t znumber;
      znumber = zselected_precompile;
      struct zoptionzIU64zK z2zE1270;
      CREATE(zoptionzIU64zK)(&z2zE1270);
      {
        struct zByteSliceFields z2zE1269;
        z2zE1269 = ztx.zinput_src;
        zprecompile_gas(&z2zE1270, znumber, z2zE1269, zgas_remaining);
      }
      unit z3zE1328;
      {
        if (z2zE1270.kind != Kind_zSomezIU64zK) goto case_1827;
        uint64_t zused;
        zused = z2zE1270.variants.zSomezIU64zK;
        struct zPrecompileResult zresult;
        {
          struct zByteSliceFields z2zE1275;
          z2zE1275 = ztx.zinput_src;
          zresult = zrun_precompile_slice(znumber, z2zE1275);
        }
        bool z2zE1271;
        z2zE1271 = zresult.zsuccess;
        if (z2zE1271) {
          {
            zgas_remaining = zgas_sub_or_oog(zgas_remaining, zused);
            if (have_exception) {
              KILL(zoptionzIU64zK)(&z2zE1270);
              goto end_block_exception_1829;
            }
          }
          unit z3zE1331;
          z3zE1331 = UNIT;
          bool z2zE1272;
          z2zE1272 = zis_running(UNIT);
          if (z2zE1272) {
            struct zHaltKind z2zE1274;
            CREATE(zHaltKind)(&z2zE1274);
            {
              struct zByteSliceFields z2zE1273;
              z2zE1273 = zresult.zoutput;
              zHaltReturn(&z2zE1274, z2zE1273);
            }
            zHalted(&zframe_status, z2zE1274);
            z3zE1328 = UNIT;
            KILL(zHaltKind)(&z2zE1274);
          } else {  z3zE1328 = UNIT;  }
        } else {
          {
            z3zE1328 = zexc_halt(zOutOfGas);
            if (have_exception) {
              KILL(zoptionzIU64zK)(&z2zE1270);
              goto end_block_exception_1829;
            }
          }
        }
        goto finish_match_1825;
      }
    case_1827: ;
      {
        {
          z3zE1328 = zexc_halt(zOutOfGas);
          if (have_exception) {
            KILL(zoptionzIU64zK)(&z2zE1270);
            goto end_block_exception_1829;
          }
        }
        goto finish_match_1825;
      }
    case_1826: ;
    finish_match_1825: ;
      z3zE1311 = z3zE1328;
      KILL(zoptionzIU64zK)(&z2zE1270);
    } else {
      bool z2zE1276;
      z2zE1276 = zfork_lt(zk_fork, zAmsterdam);
      unit z3zE1315;
      if (z2zE1276) {
        zcalldata = ztx.zinput_src;
        unit z3zE1318;
        z3zE1318 = UNIT;
        sail_fixed_bytes_20 z2zE1277;
        z2zE1277 = ztx.zrecipient;
        sail_fixed_bytes_20 z2zE1278;
        z2zE1278 = ztx.zrecipient;
        sail_u256 z2zE1279;
        z2zE1279 = ztx.zvalue;
        struct zMessage z3zE1316;
        z3zE1316.zaddress = z2zE1277;
        z3zE1316.zcaller = zsender;
        z3zE1316.zcode_address = z2zE1278;
        z3zE1316.zdepth = UINT64_C(0);
        z3zE1316.zis_static = false;
        z3zE1316.zstate_gas_reservoir = zstate_gas_remaining;
        z3zE1316.zvalue = z2zE1279;
        zmessage = z3zE1316;
        unit z3zE1317;
        z3zE1317 = UNIT;
        struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1281;
        {
          sail_fixed_bytes_20 z2zE1280;
          z2zE1280 = ztx.zrecipient;
          {
            z2zE1281 = zk_deleg_target(z2zE1280);
            if (have_exception) {  goto end_block_exception_1829;  }
          }
        }
        unit z3zE1319;
        {
          bool ztx_deleg;
          ztx_deleg = z2zE1281.ztup0;
          sail_fixed_bytes_20 ztx_dtgt;
          ztx_dtgt = z2zE1281.ztup1;
          unit z3zE1320;
          if (ztx_deleg) {
            bool z2zE1282;
            z2zE1282 = zk_access_account(ztx_dtgt);
            unit z3zE1321;
            {
              struct zAccount z2zE1283;
              {
                z2zE1283 = zk_aload(ztx_dtgt);
                if (have_exception) {  goto end_block_exception_1829;  }
              }
              unit z3zE1322;
              {
                z3zE1322 = UNIT;
                goto finish_match_1821;
              }
            case_1822: ;
              sail_match_failure("run_call_transaction_frame");
            finish_match_1821: ;
              z3zE1321 = z3zE1322;
              goto finish_match_1819;
            }
          case_1820: ;
            sail_match_failure("run_call_transaction_frame");
          finish_match_1819: ;
            z3zE1320 = z3zE1321;
          } else {  z3zE1320 = UNIT;  }
          sail_fixed_bytes_20 z2zE1284;
          z2zE1284 = ztx.zrecipient;
          {
            zframe_code = zexecutable_code(z2zE1284, ztx_deleg, ztx_dtgt);
            if (have_exception) {  goto end_block_exception_1829;  }
          }
          z3zE1319 = UNIT;
          goto finish_match_1817;
        }
      case_1818: ;
        sail_match_failure("run_call_transaction_frame");
      finish_match_1817: ;
        z3zE1315 = z3zE1319;
      } else {  z3zE1315 = UNIT;  }
      struct zByteSliceFields z2zE1285;
      z2zE1285 = zinterpret(UNIT);
      unit z3zE1326;
      {
        z3zE1326 = UNIT;
        goto finish_match_1823;
      }
    case_1824: ;
      sail_match_failure("run_call_transaction_frame");
    finish_match_1823: ;
      z3zE1311 = z3zE1326;
    }
    goto finish_match_1815;
  }
case_1816: ;
  sail_match_failure("run_call_transaction_frame");
finish_match_1815: ;
  z8zE516 = z3zE1311;
end_function_1828: ;
  return z8zE516;
end_block_exception_1829: ;

  return UNIT;
}

struct zTxFrameResult zrun_legacy_transaction_frame(struct zTransaction ztx, struct zTxValidity zv)
{
  struct zTxFrameResult z8zE517;
  uint64_t zcheckpoint;
  zcheckpoint = zk_state_checkpoint(UNIT);
  uint64_t z2zE1251;
  z2zE1251 = zv.zgas_limit;
  uint64_t z2zE1252;
  z2zE1252 = zv.zintrinsic_execution_gas;
  uint64_t z2zE1253;
  z2zE1253 = zv.zintrinsic_state_gas;
  unit z3zE1308;
  {
    z3zE1308 = zenter_transaction_frame(z2zE1251, z2zE1252, z2zE1253);
    if (have_exception) {  goto end_block_exception_1814;  }
  }
  bool z2zE1254;
  z2zE1254 = ztx.zis_create;
  unit z3zE1307;
  if (z2zE1254) {
    sail_fixed_bytes_20 z2zE1255;
    z2zE1255 = zv.zsender;
    uint64_t z2zE1256;
    z2zE1256 = zv.znonce_before;
    {
      z3zE1307 = zrun_create_transaction_frame(ztx, z2zE1255, z2zE1256);
      if (have_exception) {  goto end_block_exception_1814;  }
    }
  } else {
    sail_fixed_bytes_20 z2zE1257;
    z2zE1257 = zv.zsender;
    {
      z3zE1307 = zrun_call_transaction_frame(ztx, z2zE1257, false);
      if (have_exception) {  goto end_block_exception_1814;  }
    }
  }
  bool zsuccess;
  zsuccess = zframe_succeeded(UNIT);
  bool z2zE1258;
  z2zE1258 = not(zsuccess);
  unit z3zE1309;
  if (z2zE1258) {  z3zE1309 = zk_revert(zcheckpoint);  } else {  z3zE1309 = UNIT;  }
  __int128 z2zE1259;
  z2zE1259 = zframe_state_gas_used(UNIT);
  __int128 z2zE1260;
  if (zsuccess) {  z2zE1260 = zframe_refund;  } else {  z2zE1260 = zGAS_REFUND_ZERO;  }
  struct zTxFrameResult z3zE1310;
  z3zE1310.zexecution_gas_remaining = zgas_remaining;
  z3zE1310.zrefund = z2zE1260;
  z3zE1310.zstate_gas_remaining = zstate_gas_remaining;
  z3zE1310.zstate_gas_used = z2zE1259;
  z3zE1310.zsuccess = zsuccess;
  z8zE517 = z3zE1310;
end_function_1813: ;
  return z8zE517;
end_block_exception_1814: ;
  struct zTxFrameResult z8zE895 = { .zexecution_gas_remaining = UINT64_C(0xdeadc0de), .zrefund = ((__int128)INT64_C(0xdeadc0de)), .zstate_gas_remaining = UINT64_C(0xdeadc0de), .zstate_gas_used = ((__int128)INT64_C(0xdeadc0de)), .zsuccess = false };
  return z8zE895;
}

struct zTxFrameResult zrun_amsterdam_transaction_frame(struct zTransaction ztx, struct zTxValidity zv, struct zTxUpfrontResult zupfront)
{
  struct zTxFrameResult z8zE518;
  uint64_t z2zE1229;
  z2zE1229 = zv.zgas_limit;
  uint64_t z2zE1230;
  z2zE1230 = zv.zintrinsic_execution_gas;
  uint64_t z2zE1231;
  z2zE1231 = zv.zintrinsic_state_gas;
  unit z3zE1291;
  {
    z3zE1291 = zenter_transaction_frame(z2zE1229, z2zE1230, z2zE1231);
    if (have_exception) {  goto end_block_exception_1812;  }
  }
  uint64_t zpreparation_checkpoint;
  zpreparation_checkpoint = zk_state_checkpoint(UNIT);
  uint64_t zpreparation_reservoir;
  zpreparation_reservoir = zstate_gas_remaining;
  sail_fixed_bytes_20 zcurrent_target;
  {
    bool z2zE1248;
    z2zE1248 = ztx.zis_create;
    if (z2zE1248) {
      sail_fixed_bytes_20 z2zE1249;
      z2zE1249 = zv.zsender;
      uint64_t z2zE1250;
      z2zE1250 = zv.znonce_before;
      zcurrent_target = zk_create_addr(z2zE1249, z2zE1250);
    } else {  zcurrent_target = ztx.zrecipient;  }
  }
  struct zAmsterdamAuthorizzationState z2zE1236;
  CREATE(zAmsterdamAuthorizzationState)(&z2zE1236);
  {
    zz5listz8z5structz0zzAuthorizzzzationz9 z2zE1233;
    CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE1233);
    COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE1233, ztx.zauthorizzations);
    sail_fixed_bytes_20 z2zE1234;
    z2zE1234 = zv.zsender;
    bool z2zE1235;
    {
      sail_u256 z2zE1232;
      z2zE1232 = ztx.zvalue;
      z2zE1235 = zword_nonzzero(z2zE1232);
    }
    {
      zprocess_amsterdam_auth_list(&z2zE1236, z2zE1233, z2zE1234, zcurrent_target, z2zE1235, zEMPTY_AMSTERDAM_AUTHORIZATION_STATE);
      if (have_exception) {
        KILL(zAmsterdamAuthorizzationState)(&z2zE1236);
        KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE1233);
        goto end_block_exception_1812;
      }
    }
    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE1233);
  }
  struct zTxFrameResult z3zE1292;
  {
    __int128 zauthorizzation_state_gas;
    zauthorizzation_state_gas = zFRAME_STATE_GAS_DELTA_ZERO;
    bool zdelegated;
    zdelegated = false;
    bool z2zE1237;
    z2zE1237 = zis_running(UNIT);
    unit z3zE1299;
    if (z2zE1237) {
      zauthorizzation_state_gas = zframe_state_gas_used(UNIT);
      unit z3zE1302;
      z3zE1302 = UNIT;
      zmessage.zstate_gas_reservoir = zstate_gas_remaining;
      unit z3zE1301;
      z3zE1301 = UNIT;
      zstate_gas_spilled = zSTATE_GAS_SPILL_ZERO;
      unit z3zE1300;
      z3zE1300 = UNIT;
      {
        zdelegated = zprepare_amsterdam_transaction_dispatch(ztx, zv, zupfront);
        if (have_exception) {
          KILL(zAmsterdamAuthorizzationState)(&z2zE1236);
          goto end_block_exception_1812;
        }
      }
      z3zE1299 = UNIT;
    } else {  z3zE1299 = UNIT;  }
    bool z2zE1239;
    {
      bool z2zE1238;
      z2zE1238 = zis_running(UNIT);
      z2zE1239 = not(z2zE1238);
    }
    unit z3zE1293;
    if (z2zE1239) {
      unit z3zE1297;
      z3zE1297 = zk_revert(zpreparation_checkpoint);
      zmessage.zstate_gas_reservoir = zpreparation_reservoir;
      unit z3zE1296;
      z3zE1296 = UNIT;
      zstate_gas_remaining = zpreparation_reservoir;
      unit z3zE1295;
      z3zE1295 = UNIT;
      zstate_gas_spilled = zSTATE_GAS_SPILL_ZERO;
      unit z3zE1294;
      z3zE1294 = UNIT;
      struct zTxFrameResult z3zE1298;
      z3zE1298.zexecution_gas_remaining = zgas_remaining;
      z3zE1298.zrefund = zGAS_REFUND_ZERO;
      z3zE1298.zstate_gas_remaining = zstate_gas_remaining;
      z3zE1298.zstate_gas_used = zSTATE_GAS_DELTA_ZERO;
      z3zE1298.zsuccess = false;
      z8zE518 = z3zE1298;
      goto cleanup_1810;
      /* unreachable after return */
      goto end_cleanup_1811;
    cleanup_1810: ;
      KILL(zAmsterdamAuthorizzationState)(&z2zE1236);
      goto end_function_1809;
    end_cleanup_1811: ;
    } else {  z3zE1293 = UNIT;  }
    uint64_t zexecution_checkpoint;
    zexecution_checkpoint = zk_state_checkpoint(UNIT);
    bool z2zE1240;
    z2zE1240 = ztx.zis_create;
    unit z3zE1303;
    if (z2zE1240) {
      sail_fixed_bytes_20 z2zE1241;
      z2zE1241 = zv.zsender;
      uint64_t z2zE1242;
      z2zE1242 = zv.znonce_before;
      {
        z3zE1303 = zrun_create_transaction_frame(ztx, z2zE1241, z2zE1242);
        if (have_exception) {
          KILL(zAmsterdamAuthorizzationState)(&z2zE1236);
          goto end_block_exception_1812;
        }
      }
    } else {
      sail_fixed_bytes_20 z2zE1243;
      z2zE1243 = zv.zsender;
      {
        z3zE1303 = zrun_call_transaction_frame(ztx, z2zE1243, zdelegated);
        if (have_exception) {
          KILL(zAmsterdamAuthorizzationState)(&z2zE1236);
          goto end_block_exception_1812;
        }
      }
    }
    bool zsuccess;
    zsuccess = zframe_succeeded(UNIT);
    bool z2zE1244;
    z2zE1244 = not(zsuccess);
    unit z3zE1304;
    if (z2zE1244) {  z3zE1304 = zk_revert(zexecution_checkpoint);  } else {  z3zE1304 = UNIT;  }
    __int128 z2zE1246;
    {
      __int128 z2zE1245;
      z2zE1245 = zframe_state_gas_used(UNIT);
      {    z2zE1246 = (zauthorizzation_state_gas + z2zE1245);
      }
    }
    __int128 z2zE1247;
    if (zsuccess) {  z2zE1247 = zframe_refund;  } else {  z2zE1247 = zGAS_REFUND_ZERO;  }
    struct zTxFrameResult z3zE1305;
    z3zE1305.zexecution_gas_remaining = zgas_remaining;
    z3zE1305.zrefund = z2zE1247;
    z3zE1305.zstate_gas_remaining = zstate_gas_remaining;
    z3zE1305.zstate_gas_used = z2zE1246;
    z3zE1305.zsuccess = zsuccess;
    z3zE1292 = z3zE1305;
    goto finish_match_1807;
  }
case_1808: ;
  sail_match_failure("run_amsterdam_transaction_frame");
finish_match_1807: ;
  z8zE518 = z3zE1292;
  KILL(zAmsterdamAuthorizzationState)(&z2zE1236);
end_function_1809: ;
  return z8zE518;
end_block_exception_1812: ;
  struct zTxFrameResult z8zE896 = { .zexecution_gas_remaining = UINT64_C(0xdeadc0de), .zrefund = ((__int128)INT64_C(0xdeadc0de)), .zstate_gas_remaining = UINT64_C(0xdeadc0de), .zstate_gas_used = ((__int128)INT64_C(0xdeadc0de)), .zsuccess = false };
  return z8zE896;
}

struct zTxFrameResult zrun_transaction_frame(struct zTransaction ztx, struct zTxValidity zv, struct zTxUpfrontResult zupfront)
{
  struct zTxFrameResult z8zE519;
  bool z2zE1228;
  z2zE1228 = zfork_gteq(zk_fork, zAmsterdam);
  if (z2zE1228) {
    {
      z8zE519 = zrun_amsterdam_transaction_frame(ztx, zv, zupfront);
      if (have_exception) {  goto end_block_exception_1806;  }
    }
  } else {
    {
      z8zE519 = zrun_legacy_transaction_frame(ztx, zv);
      if (have_exception) {  goto end_block_exception_1806;  }
    }
  }
end_function_1805: ;
  return z8zE519;
end_block_exception_1806: ;
  struct zTxFrameResult z8zE897 = { .zexecution_gas_remaining = UINT64_C(0xdeadc0de), .zrefund = ((__int128)INT64_C(0xdeadc0de)), .zstate_gas_remaining = UINT64_C(0xdeadc0de), .zstate_gas_used = ((__int128)INT64_C(0xdeadc0de)), .zsuccess = false };
  return z8zE897;
}

uint64_t zadmitted_transaction_state_gas(__int128 zvalue)
{
  uint64_t z8zE520;
  bool z2zE1222;
  z2zE1222 = (!(UINT64_C(0) < zvalue));
  if (z2zE1222) {  z8zE520 = zGAS_ZERO;  } else {
    bool z2zE1223;
    z2zE1223 = (!(UINT64_C(18446744073709551615) < zvalue));
    if (z2zE1223) {  z8zE520 = (uint64_t)(zvalue);  } else {
      struct zexception z2zE1224;
      CREATE(zexception)(&z2zE1224);
      zInvalidBlock(&z2zE1224, zExecutionInvalid);
      COPY(zexception)(current_exception, z2zE1224);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/evm/transaction.sail:1049.12-1049.48");
      KILL(zexception)(&z2zE1224);
      goto end_block_exception_1802;
      /* unreachable after throw */
      KILL(zexception)(&z2zE1224);
    }
  }
end_function_1801: ;
  return z8zE520;
end_block_exception_1802: ;

  return UINT64_C(0xdeadc0de);
}

void zsettle_transaction(struct zReceipt *z8zE521, struct zTransaction ztx, struct zTxValidity zv, __int128 zauthorizzation_refund, struct zTxFrameResult zfr)
{
  uint64_t zgas_left0;
  {
    uint64_t z2zE1220;
    z2zE1220 = zfr.zexecution_gas_remaining;
    uint64_t z2zE1221;
    z2zE1221 = zfr.zstate_gas_remaining;
    {
      zgas_left0 = zvalidated_gas_add(z2zE1220, z2zE1221);
      if (have_exception) {  goto end_block_exception_1800;  }
    }
  }
  uint64_t zgas_used0;
  {
    uint64_t z2zE1219;
    z2zE1219 = zv.zgas_limit;
    {
      zgas_used0 = zvalidated_gas_subzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE1219, zgas_left0);
      if (have_exception) {  goto end_block_exception_1800;  }
    }
  }
  uint64_t zrefund_quotient;
  {
    bool z2zE1218;
    z2zE1218 = zfork_gteq(zk_fork, zLondon);
    if (z2zE1218) {  zrefund_quotient = UINT64_C(5);  } else {  zrefund_quotient = UINT64_C(2);  }
  }
  uint64_t zrefund_cap;
  zrefund_cap = (zgas_used0 / zrefund_quotient);
  __int128 ztotal_refund;
  {
    __int128 z2zE1217;
    z2zE1217 = zfr.zrefund;
    {
      ztotal_refund = zvalidated_refund_add(zauthorizzation_refund, z2zE1217);
      if (have_exception) {  goto end_block_exception_1800;  }
    }
  }
  uint64_t zrefund;
  zrefund = zcapped_transaction_refundzIreprzGI128zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(ztotal_refund, zrefund_cap);
  uint64_t zgas_left1;
  {
    zgas_left1 = zvalidated_gas_add(zgas_left0, zrefund);
    if (have_exception) {  goto end_block_exception_1800;  }
  }
  uint64_t zgas_used1;
  {
    uint64_t z2zE1216;
    z2zE1216 = zv.zgas_limit;
    {
      zgas_used1 = zvalidated_gas_subzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE1216, zgas_left1);
      if (have_exception) {  goto end_block_exception_1800;  }
    }
  }
  uint64_t zfloor;
  {
    bool z2zE1213;
    z2zE1213 = zfork_gteq(zk_fork, zPrague);
    if (z2zE1213) {
      uint64_t zfloor_cost;
      zfloor_cost = zv.zcalldata_floor;
      uint64_t ztx_limit;
      ztx_limit = zv.zgas_limit;
      bool z2zE1214;
      z2zE1214 = (!(ztx_limit < zfloor_cost));
      if (z2zE1214) {  zfloor = zfloor_cost;  } else {
        struct zexception z2zE1215;
        CREATE(zexception)(&z2zE1215);
        zInvalidBlock(&z2zE1215, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1215);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:1084.16-1084.52");
        KILL(zexception)(&z2zE1215);
        goto end_block_exception_1800;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1215);
      }
    } else {  zfloor = UINT64_C(0);  }
  }
  uint64_t zgas_used;
  {
    bool z2zE1212;
    z2zE1212 = (zgas_used1 < zfloor);
    if (z2zE1212) {  zgas_used = zfloor;  } else {  zgas_used = zgas_used1;  }
  }
  uint64_t zgas_left;
  {
    uint64_t z2zE1211;
    z2zE1211 = zv.zgas_limit;
    {
      zgas_left = zvalidated_gas_subzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE1211, zgas_used);
      if (have_exception) {  goto end_block_exception_1800;  }
    }
  }
  __int128 zraw_state_gas;
  {
    __int128 z2zE1209;
    z2zE1209 = zfr.zstate_gas_used;
    uint64_t z2zE1210;
    z2zE1210 = zv.zintrinsic_state_gas;
    {
      __int128 z3zE1285;
      z3zE1285 = (__int128)(z2zE1210);
      zraw_state_gas = (z2zE1209 + z3zE1285);
    }
  }
  uint64_t ztx_state_gas;
  {
    ztx_state_gas = zadmitted_transaction_state_gas(zraw_state_gas);
    if (have_exception) {  goto end_block_exception_1800;  }
  }
  uint64_t zexecution_before_floor;
  zexecution_before_floor = zGAS_ZERO;
  bool z2zE1196;
  z2zE1196 = (!(zgas_used0 < ztx_state_gas));
  unit z3zE1286;
  if (z2zE1196) {
    uint64_t zreduced_execution_gas;
    {
      zreduced_execution_gas = zvalidated_gas_subzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zgas_used0, ztx_state_gas);
      if (have_exception) {  goto end_block_exception_1800;  }
    }
    zexecution_before_floor = zreduced_execution_gas;
    z3zE1286 = UNIT;
  } else {  z3zE1286 = UNIT;  }
  uint64_t zexecution_gas;
  {
    bool z2zE1207;
    z2zE1207 = zfork_gteq(zk_fork, zAmsterdam);
    if (z2zE1207) {
      bool z2zE1208;
      z2zE1208 = (zexecution_before_floor < zfloor);
      if (z2zE1208) {  zexecution_gas = zfloor;  } else {  zexecution_gas = zexecution_before_floor;  }
    } else {  zexecution_gas = zgas_used;  }
  }
  uint64_t zstate_gas;
  {
    bool z2zE1206;
    z2zE1206 = zfork_gteq(zk_fork, zAmsterdam);
    if (z2zE1206) {  zstate_gas = ztx_state_gas;  } else {  zstate_gas = zGAS_ZERO;  }
  }
  sail_fixed_bytes_20 z2zE1198;
  z2zE1198 = zv.zsender;
  sail_u256 z2zE1199;
  {
    sail_u256 z2zE1197;
    z2zE1197 = zv.zgas_price;
    {
      z2zE1199 = zvalidated_word_productzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE1197, zgas_left);
      if (have_exception) {  goto end_block_exception_1800;  }
    }
  }
  unit z3zE1289;
  {
    z3zE1289 = zk_add_balance(z2zE1198, z2zE1199);
    if (have_exception) {  goto end_block_exception_1800;  }
  }
  sail_fixed_bytes_20 z2zE1201;
  z2zE1201 = zk_coinbase(UNIT);
  sail_u256 z2zE1202;
  {
    sail_u256 z2zE1200;
    z2zE1200 = zv.zpriority_fee;
    {
      z2zE1202 = zvalidated_word_productzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE1200, zgas_used);
      if (have_exception) {  goto end_block_exception_1800;  }
    }
  }
  unit z3zE1288;
  {
    z3zE1288 = zk_add_balance(z2zE1201, z2zE1202);
    if (have_exception) {  goto end_block_exception_1800;  }
  }
  unit z3zE1287;
  z3zE1287 = zk_tx_merge(UNIT);
  enum zTxType z2zE1203;
  z2zE1203 = ztx.ztx_type;
  bool z2zE1204;
  z2zE1204 = zfr.zsuccess;
  zz5listz8z5structz0zzLogEntryz9 z2zE1205;
  CREATE(zz5listz8z5structz0zzLogEntryz9)(&z2zE1205);
  logs_read_all(&z2zE1205, UNIT);
  struct zReceipt z3zE1290;
  CREATE(zReceipt)(&z3zE1290);
  z3zE1290.zexecution_gas = zexecution_gas;
  z3zE1290.zgas_used = zgas_used;
  COPY(zz5listz8z5structz0zzLogEntryz9)(&((&z3zE1290)->zlogs), z2zE1205);
  z3zE1290.zstate_gas = zstate_gas;
  z3zE1290.zsuccess = z2zE1204;
  z3zE1290.ztx_type = z2zE1203;
  COPY(zReceipt)((*(&z8zE521)), z3zE1290);
  KILL(zReceipt)(&z3zE1290);
  KILL(zz5listz8z5structz0zzLogEntryz9)(&z2zE1205);
end_function_1799: ;
  goto end_function_3632;
end_block_exception_1800: ;
  goto end_function_3632;
end_function_3632: ;
}

void zprocess_transaction(struct zReceipt *z8zE522, struct zTransaction ztx)
{
  unit z3zE1275;
  z3zE1275 = zcycle_scope_start(zSCOPE_TX_RESET);
  unit z3zE1274;
  z3zE1274 = zk_tx_reset(UNIT);
  unit z3zE1273;
  z3zE1273 = zcycle_scope_end(zSCOPE_TX_RESET);
  unit z3zE1272;
  z3zE1272 = zcycle_scope_start(zSCOPE_TX_VALIDATE);
  struct zTxValidity zvalidity;
  {
    zvalidity = zcheck_transaction_validity(ztx);
    if (have_exception) {  goto end_block_exception_1798;  }
  }
  unit z3zE1279;
  z3zE1279 = zcycle_scope_end(zSCOPE_TX_VALIDATE);
  struct zTxEnv z2zE1194;
  {
    sail_fixed_bytes_20 z2zE1191;
    z2zE1191 = ztx.zsender;
    sail_u256 z2zE1192;
    z2zE1192 = zvalidity.zgas_price;
    struct zBlobHashes z2zE1193;
    z2zE1193 = ztx.zblob_hashes;
    struct zTxEnv z3zE1276;
    z3zE1276.zblob_hashes = z2zE1193;
    z3zE1276.zgas_price = z2zE1192;
    z3zE1276.zorigin = z2zE1191;
    z2zE1194 = z3zE1276;
  }
  unit z3zE1278;
  z3zE1278 = zk_set_tx(z2zE1194);
  unit z3zE1277;
  z3zE1277 = zcycle_scope_start(zSCOPE_TX_UPFRONT);
  struct zTxUpfrontResult zupfront;
  {
    zupfront = zapply_transaction_upfront_effects(ztx, zvalidity);
    if (have_exception) {  goto end_block_exception_1798;  }
  }
  unit z3zE1281;
  z3zE1281 = zcycle_scope_end(zSCOPE_TX_UPFRONT);
  unit z3zE1280;
  z3zE1280 = zcycle_scope_start(zSCOPE_TX_FRAME);
  struct zTxFrameResult zframe_result;
  {
    zframe_result = zrun_transaction_frame(ztx, zvalidity, zupfront);
    if (have_exception) {  goto end_block_exception_1798;  }
  }
  unit z3zE1283;
  z3zE1283 = zcycle_scope_end(zSCOPE_TX_FRAME);
  unit z3zE1282;
  z3zE1282 = zcycle_scope_start(zSCOPE_TX_SETTLE);
  struct zReceipt zreceipt;
  CREATE(zReceipt)(&zreceipt);
  {
    __int128 z2zE1195;
    z2zE1195 = zupfront.zauthorizzation_refund;
    {
      zsettle_transaction(&zreceipt, ztx, zvalidity, z2zE1195, zframe_result);
      if (have_exception) {
        KILL(zReceipt)(&zreceipt);
        goto end_block_exception_1798;
      }
    }
  }
  unit z3zE1284;
  z3zE1284 = zcycle_scope_end(zSCOPE_TX_SETTLE);
  COPY(zReceipt)((*(&z8zE522)), zreceipt);
  KILL(zReceipt)(&zreceipt);
end_function_1797: ;
  goto end_function_3631;
end_block_exception_1798: ;
  goto end_function_3631;
end_function_3631: ;
}

