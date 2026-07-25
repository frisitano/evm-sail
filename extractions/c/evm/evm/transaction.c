/* Generated from sail/evm/transaction.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void create_letbind_198(void) {

  uint64_t z3zE251;
  z3zE251 = UINT64_C(21000);
  zG_transaction = z3zE251;
let_end_669: ;
}
void kill_letbind_198(void) {
}

void create_letbind_199(void) {

  uint64_t z3zE252;
  z3zE252 = UINT64_C(32000);
  zG_txcreate = z3zE252;
let_end_670: ;
}
void kill_letbind_199(void) {
}

void create_letbind_200(void) {

  uint64_t z3zE253;
  z3zE253 = UINT64_C(4);
  zG_txdatazzero = z3zE253;
let_end_671: ;
}
void kill_letbind_200(void) {
}

void create_letbind_201(void) {

  uint64_t z3zE254;
  z3zE254 = UINT64_C(16);
  zG_txdatanonzzero = z3zE254;
let_end_672: ;
}
void kill_letbind_201(void) {
}

void create_letbind_202(void) {

  uint64_t z3zE255;
  z3zE255 = UINT64_C(2400);
  zG_access_list_address = z3zE255;
let_end_673: ;
}
void kill_letbind_202(void) {
}

void create_letbind_203(void) {

  uint64_t z3zE256;
  z3zE256 = UINT64_C(1900);
  zG_access_list_storage_key = z3zE256;
let_end_674: ;
}
void kill_letbind_203(void) {
}

void create_letbind_204(void) {

  uint64_t z3zE257;
  z3zE257 = UINT64_C(12500);
  zPER_AUTH_BASE = z3zE257;
let_end_675: ;
}
void kill_letbind_204(void) {
}

void create_letbind_205(void) {

  uint64_t z3zE258;
  z3zE258 = UINT64_C(25000);
  zPER_EMPTY_ACCOUNT = z3zE258;
let_end_676: ;
}
void kill_letbind_205(void) {
}

void create_letbind_206(void) {

  uint64_t z3zE259;
  z3zE259 = UINT64_C(12000);
  zAMSTERDAM_TX_BASE = z3zE259;
let_end_677: ;
}
void kill_letbind_206(void) {
}

void create_letbind_207(void) {

  uint64_t z3zE260;
  z3zE260 = UINT64_C(11000);
  zAMSTERDAM_CREATE_ACCESS = z3zE260;
let_end_678: ;
}
void kill_letbind_207(void) {
}

void create_letbind_208(void) {

  uint64_t z3zE261;
  z3zE261 = UINT64_C(3000);
  zAMSTERDAM_COLD_ACCOUNT_ACCESS = z3zE261;
let_end_679: ;
}
void kill_letbind_208(void) {
}

void create_letbind_209(void) {

  uint64_t z3zE262;
  z3zE262 = UINT64_C(4244);
  zAMSTERDAM_TX_VALUE_COST = z3zE262;
let_end_680: ;
}
void kill_letbind_209(void) {
}

void create_letbind_210(void) {

  uint64_t z3zE263;
  z3zE263 = UINT64_C(1756);
  zAMSTERDAM_TRANSFER_LOG_COST = z3zE263;
let_end_681: ;
}
void kill_letbind_210(void) {
}

void create_letbind_211(void) {

  uint64_t z3zE264;
  z3zE264 = UINT64_C(3000);
  zAMSTERDAM_ACCESS_LIST_ADDRESS = z3zE264;
let_end_682: ;
}
void kill_letbind_211(void) {
}

void create_letbind_212(void) {

  uint64_t z3zE265;
  z3zE265 = UINT64_C(3000);
  zAMSTERDAM_ACCESS_LIST_SLOT = z3zE265;
let_end_683: ;
}
void kill_letbind_212(void) {
}

void create_letbind_213(void) {

  uint64_t z3zE266;
  z3zE266 = UINT64_C(1280);
  zAMSTERDAM_ACCESS_LIST_ADDRESS_FLOOR = z3zE266;
let_end_684: ;
}
void kill_letbind_213(void) {
}

void create_letbind_214(void) {

  uint64_t z3zE267;
  z3zE267 = UINT64_C(2048);
  zAMSTERDAM_ACCESS_LIST_SLOT_FLOOR = z3zE267;
let_end_685: ;
}
void kill_letbind_214(void) {
}

void create_letbind_215(void) {

  uint64_t z3zE268;
  z3zE268 = UINT64_C(7816);
  zAMSTERDAM_AUTH_BASE = z3zE268;
let_end_686: ;
}
void kill_letbind_215(void) {
}

void create_letbind_216(void) {

  uint64_t z3zE269;
  z3zE269 = UINT64_C(64);
  zAMSTERDAM_CALLDATA_FLOOR_BYTE = z3zE269;
let_end_687: ;
}
void kill_letbind_216(void) {
}

void create_letbind_217(void) {

  uint64_t z3zE270;
  z3zE270 = zTRANSACTION_EXECUTION_GAS_LIMIT;
  zAMSTERDAM_TX_MAX_GAS = z3zE270;
let_end_688: ;
}
void kill_letbind_217(void) {
}

struct zByteSliceFields ztransaction_initcode_slice(struct zByteSliceFields zinput)
{
  struct zByteSliceFields z8zE446;
  z8zE446 = zcode_slice(zinput);
end_function_2161: ;
  return z8zE446;
end_block_exception_2162: ;
  struct zByteSliceFields z8zE973 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE973;
}

void create_letbind_218(void) {    CREATE(zAmsterdamAuthorizzationState)(&zEMPTY_AMSTERDAM_AUTHORIZATION_STATE);

  struct zAmsterdamAuthorizzationState z3zE275;
  CREATE(zAmsterdamAuthorizzationState)(&z3zE275);
  struct zAmsterdamAuthorizzationState z3zE271;
  CREATE(zAmsterdamAuthorizzationState)(&z3zE271);
  zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z3zE272;
  CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE272);
  COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE271)->zdelegation_set_for), z3zE272);
  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE272);
  zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z3zE273;
  CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE273);
  COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE271)->zoriginally_delegated), z3zE273);
  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE273);
  zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z3zE274;
  CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE274);
  COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE271)->zseen_valid_authorities), z3zE274);
  KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z3zE274);
  COPY(zAmsterdamAuthorizzationState)(&z3zE275, z3zE271);
  KILL(zAmsterdamAuthorizzationState)(&z3zE271);
  COPY(zAmsterdamAuthorizzationState)(&zEMPTY_AMSTERDAM_AUTHORIZATION_STATE, z3zE275);
  KILL(zAmsterdamAuthorizzationState)(&z3zE275);
let_end_690: ;
}
void kill_letbind_218(void) {    KILL(zAmsterdamAuthorizzationState)(&zEMPTY_AMSTERDAM_AUTHORIZATION_STATE);
}

uint64_t zcalldata_cost(struct zByteSliceFields zinput)
{
  uint64_t z8zE447;
  uint64_t znonzzeroes;
  znonzzeroes = zslice_count_nonzzero(zinput);
  uint64_t zinput_len;
  zinput_len = zinput.zlen;
  bool z2zE2030;
  z2zE2030 = (!(zinput_len < znonzzeroes));
  if (z2zE2030) {
    uint64_t zzzeroes;
    {    zzzeroes = (zinput_len - znonzzeroes);
    }
    uint64_t z2zE2031;
    {    z2zE2031 = (zG_txdatazzero * zzzeroes);
    }
    uint64_t z2zE2032;
    {    z2zE2032 = (zG_txdatanonzzero * znonzzeroes);
    }
    {    z8zE447 = (z2zE2031 + z2zE2032);
    }
  } else {
    struct zexception z2zE2033;
    CREATE(zexception)(&z2zE2033);
    zInvalidBlock(&z2zE2033, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE2033);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/transaction.sail:105.8-105.44");
    KILL(zexception)(&z2zE2033);
    goto end_block_exception_2160;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2033);
  }
end_function_2159: ;
  return z8zE447;
end_block_exception_2160: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zvalidate_blob_hash_version_at(struct zBlobHashes zhashes, uint64_t zremaining, uint64_t zoffset)
{
  uint64_t z8zE448;
  bool z2zE2025;
  z2zE2025 = (zremaining == UINT64_C(0));
  if (z2zE2025) {  z8zE448 = UINT64_C(0);  } else {
    bool z2zE2028;
    {
      uint64_t z2zE2027;
      {
        struct zByteSliceFields z2zE2026;
        z2zE2026 = zhashes.zbytes;
        z2zE2027 = zslice_byte(z2zE2026, zoffset);
      }
      z2zE2028 = (z2zE2027 != UINT64_C(0x01));
    }
    unit z3zE1832;
    if (z2zE2028) {
      struct zexception z2zE2029;
      CREATE(zexception)(&z2zE2029);
      zInvalidBlock(&z2zE2029, zExecutionInvalid);
      COPY(zexception)(current_exception, z2zE2029);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/evm/transaction.sail:135.12-135.48");
      KILL(zexception)(&z2zE2029);
      goto end_block_exception_2158;
      /* unreachable after throw */
      KILL(zexception)(&z2zE2029);
    } else {  z3zE1832 = UNIT;  }
    z8zE448 = ztransaction_blob_count_decrement(zremaining);
  }
end_function_2157: ;
  return z8zE448;
end_block_exception_2158: ;

  return UINT64_C(0xdeadc0de);
}

unit zvalidate_blob_hash_versions(struct zBlobHashes zhashes)
{
  unit z8zE449;
  uint64_t zremaining;
  zremaining = zhashes.zcount;
  {
    zremaining = zvalidate_blob_hash_version_at(zhashes, zremaining, UINT64_C(1));
    if (have_exception) {  goto end_block_exception_2156;  }
  }
  unit z3zE1831;
  z3zE1831 = UNIT;
  {
    zremaining = zvalidate_blob_hash_version_at(zhashes, zremaining, UINT64_C(34));
    if (have_exception) {  goto end_block_exception_2156;  }
  }
  unit z3zE1830;
  z3zE1830 = UNIT;
  {
    zremaining = zvalidate_blob_hash_version_at(zhashes, zremaining, UINT64_C(67));
    if (have_exception) {  goto end_block_exception_2156;  }
  }
  unit z3zE1829;
  z3zE1829 = UNIT;
  {
    zremaining = zvalidate_blob_hash_version_at(zhashes, zremaining, UINT64_C(100));
    if (have_exception) {  goto end_block_exception_2156;  }
  }
  unit z3zE1828;
  z3zE1828 = UNIT;
  {
    zremaining = zvalidate_blob_hash_version_at(zhashes, zremaining, UINT64_C(133));
    if (have_exception) {  goto end_block_exception_2156;  }
  }
  unit z3zE1827;
  z3zE1827 = UNIT;
  {
    zremaining = zvalidate_blob_hash_version_at(zhashes, zremaining, UINT64_C(166));
    if (have_exception) {  goto end_block_exception_2156;  }
  }
  unit z3zE1826;
  z3zE1826 = UNIT;
  {
    zremaining = zvalidate_blob_hash_version_at(zhashes, zremaining, UINT64_C(199));
    if (have_exception) {  goto end_block_exception_2156;  }
  }
  unit z3zE1825;
  z3zE1825 = UNIT;
  {
    zremaining = zvalidate_blob_hash_version_at(zhashes, zremaining, UINT64_C(232));
    if (have_exception) {  goto end_block_exception_2156;  }
  }
  unit z3zE1824;
  z3zE1824 = UNIT;
  {
    zremaining = zvalidate_blob_hash_version_at(zhashes, zremaining, UINT64_C(265));
    if (have_exception) {  goto end_block_exception_2156;  }
  }
  unit z3zE1823;
  z3zE1823 = UNIT;
  bool z2zE2023;
  z2zE2023 = (zremaining != UINT64_C(0));
  if (z2zE2023) {
    struct zexception z2zE2024;
    CREATE(zexception)(&z2zE2024);
    zInvalidBlock(&z2zE2024, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE2024);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/transaction.sail:156.8-156.44");
    KILL(zexception)(&z2zE2024);
    goto end_block_exception_2156;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2024);
  } else {  z8zE449 = UNIT;  }
end_function_2155: ;
  return z8zE449;
end_block_exception_2156: ;

  return UNIT;
}

uint64_t zlegacy_intrinsic_gas(struct zTransaction ztx)
{
  uint64_t z8zE450;
  uint64_t zdata_cost;
  {
    struct zByteSliceFields z2zE2022;
    z2zE2022 = ztx.zinput_src;
    {
      zdata_cost = zcalldata_cost(z2zE2022);
      if (have_exception) {  goto end_block_exception_2154;  }
    }
  }
  struct zByteSliceFields zinput;
  zinput = ztx.zinput_src;
  uint64_t zinput_len;
  zinput_len = zinput.zlen;
  uint64_t zaddress_cost;
  {
    uint64_t z2zE2021;
    z2zE2021 = ztx.zaccess_list_address_count;
    {    zaddress_cost = (zG_access_list_address * z2zE2021);
    }
  }
  uint64_t zslot_cost;
  {
    uint64_t z2zE2020;
    z2zE2020 = ztx.zaccess_list_slot_count;
    {    zslot_cost = (zG_access_list_storage_key * z2zE2020);
    }
  }
  uint64_t zauth_cost;
  {
    uint64_t z2zE2019;
    z2zE2019 = ztx.zauthorizzation_count;
    {    zauth_cost = (zPER_EMPTY_ACCOUNT * z2zE2019);
    }
  }
  uint64_t zcommon;
  {
    uint64_t z2zE2018;
    {
      uint64_t z2zE2017;
      {
        uint64_t z2zE2016;
        {    z2zE2016 = (zdata_cost + zG_transaction);
        }
        {    z2zE2017 = (z2zE2016 + zaddress_cost);
        }
      }
      {    z2zE2018 = (z2zE2017 + zslot_cost);
      }
    }
    {    zcommon = (z2zE2018 + zauth_cost);
    }
  }
  bool z2zE2013;
  z2zE2013 = ztx.zis_create;
  if (z2zE2013) {
    uint64_t z2zE2014;
    {    z2zE2014 = (zcommon + zG_txcreate);
    }
    uint64_t z2zE2015;
    z2zE2015 = ztransaction_initcode_gas(zinput_len);
    {    z8zE450 = (z2zE2014 + z2zE2015);
    }
  } else {  z8zE450 = zcommon;  }
end_function_2153: ;
  return z8zE450;
end_block_exception_2154: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zlegacy_calldata_floor(struct zByteSliceFields zinput)
{
  uint64_t z8zE451;
  uint64_t znonzzeroes;
  znonzzeroes = zslice_count_nonzzero(zinput);
  uint64_t zinput_len;
  zinput_len = zinput.zlen;
  bool z2zE2008;
  z2zE2008 = (!(zinput_len < znonzzeroes));
  if (z2zE2008) {
    uint64_t zzzeroes;
    {    zzzeroes = (zinput_len - znonzzeroes);
    }
    uint64_t z2zE2011;
    {
      uint64_t z2zE2009;
      {    z2zE2009 = (UINT64_C(10) * zzzeroes);
      }
      uint64_t z2zE2010;
      {    z2zE2010 = (UINT64_C(40) * znonzzeroes);
      }
      {    z2zE2011 = (z2zE2009 + z2zE2010);
      }
    }
    {    z8zE451 = (z2zE2011 + zG_transaction);
    }
  } else {
    struct zexception z2zE2012;
    CREATE(zexception)(&z2zE2012);
    zInvalidBlock(&z2zE2012, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE2012);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/transaction.sail:187.8-187.44");
    KILL(zexception)(&z2zE2012);
    goto end_block_exception_2152;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2012);
  }
end_function_2151: ;
  return z8zE451;
end_block_exception_2152: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zamsterdam_recipient_execution_cost(struct zTransaction ztx)
{
  uint64_t z8zE452;
  bool z2zE1999;
  z2zE1999 = ztx.zis_create;
  if (z2zE1999) {
    bool z2zE2001;
    {
      sail_u256 z2zE2000;
      z2zE2000 = ztx.zvalue;
      z2zE2001 = zword_nonzzero(z2zE2000);
    }
    if (z2zE2001) {
      {    z8zE452 = (zAMSTERDAM_CREATE_ACCESS + zAMSTERDAM_TRANSFER_LOG_COST);
      }
    } else {  z8zE452 = zAMSTERDAM_CREATE_ACCESS;  }
  } else {
    bool z2zE2004;
    {
      sail_fixed_bytes_20 z2zE2002;
      z2zE2002 = ztx.zrecipient;
      sail_fixed_bytes_20 z2zE2003;
      z2zE2003 = ztx.zsender;
      z2zE2004 = zneq_anythingzIR__sail_c_repr_fixed_byteszIC20zKzK(z2zE2002, z2zE2003);
    }
    if (z2zE2004) {
      bool z2zE2006;
      {
        sail_u256 z2zE2005;
        z2zE2005 = ztx.zvalue;
        z2zE2006 = zword_nonzzero(z2zE2005);
      }
      if (z2zE2006) {
        uint64_t z2zE2007;
        {    z2zE2007 = (zAMSTERDAM_COLD_ACCOUNT_ACCESS + zAMSTERDAM_TX_VALUE_COST);
        }
        {    z8zE452 = (z2zE2007 + zAMSTERDAM_TRANSFER_LOG_COST);
        }
      } else {  z8zE452 = zAMSTERDAM_COLD_ACCOUNT_ACCESS;  }
    } else {  z8zE452 = UINT64_C(0);  }
  }
end_function_2149: ;
  return z8zE452;
end_block_exception_2150: ;

  return UINT64_C(0xdeadc0de);
}

struct zIntrinsicGasCost zintrinsic_gas(struct zTransaction ztx)
{
  struct zIntrinsicGasCost z8zE453;
  bool z2zE1974;
  z2zE1974 = zfork_lt(zk_fork, zAmsterdam);
  if (z2zE1974) {
    uint64_t z2zE1975;
    {
      z2zE1975 = zlegacy_intrinsic_gas(ztx);
      if (have_exception) {  goto end_block_exception_2148;  }
    }
    uint64_t z2zE1977;
    {
      struct zByteSliceFields z2zE1976;
      z2zE1976 = ztx.zinput_src;
      {
        z2zE1977 = zlegacy_calldata_floor(z2zE1976);
        if (have_exception) {  goto end_block_exception_2148;  }
      }
    }
    struct zIntrinsicGasCost z3zE1822;
    z3zE1822.zcalldata_floor = z2zE1977;
    z3zE1822.zexecution = z2zE1975;
    z3zE1822.zstate = UINT64_C(0);
    z8zE453 = z3zE1822;
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
      uint64_t z2zE1997;
      {
        uint64_t z2zE1995;
        {
          uint64_t z2zE1993;
          {    z2zE1993 = (zAMSTERDAM_ACCESS_LIST_ADDRESS * zaddress_count);
          }
          uint64_t z2zE1994;
          {    z2zE1994 = (zAMSTERDAM_ACCESS_LIST_SLOT * zslot_count);
          }
          {    z2zE1995 = (z2zE1993 + z2zE1994);
          }
        }
        uint64_t z2zE1996;
        {    z2zE1996 = (zAMSTERDAM_ACCESS_LIST_ADDRESS_FLOOR * zaddress_count);
        }
        {    z2zE1997 = (z2zE1995 + z2zE1996);
        }
      }
      uint64_t z2zE1998;
      {    z2zE1998 = (zAMSTERDAM_ACCESS_LIST_SLOT_FLOOR * zslot_count);
      }
      {    zaccess_execution = (z2zE1997 + z2zE1998);
      }
    }
    uint64_t zauthorizzation_execution;
    {
      uint64_t z2zE1992;
      z2zE1992 = ztx.zauthorizzation_count;
      {    zauthorizzation_execution = (zAMSTERDAM_AUTH_BASE * z2zE1992);
      }
    }
    uint64_t zcreate_execution;
    {
      bool z2zE1990;
      z2zE1990 = ztx.zis_create;
      if (z2zE1990) {
        uint64_t z2zE1991;
        z2zE1991 = zinput.zlen;
        zcreate_execution = ztransaction_initcode_gas(z2zE1991);
      } else {  zcreate_execution = UINT64_C(0);  }
    }
    uint64_t zexecution;
    {
      uint64_t z2zE1989;
      {
        uint64_t z2zE1988;
        {
          uint64_t z2zE1987;
          {
            uint64_t z2zE1986;
            {
              uint64_t z2zE1985;
              {
                struct zByteSliceFields z2zE1984;
                z2zE1984 = ztx.zinput_src;
                {
                  z2zE1985 = zcalldata_cost(z2zE1984);
                  if (have_exception) {  goto end_block_exception_2148;  }
                }
              }
              {    z2zE1986 = (z2zE1985 + zAMSTERDAM_TX_BASE);
              }
            }
            {    z2zE1987 = (z2zE1986 + zrecipient);
            }
          }
          {    z2zE1988 = (z2zE1987 + zaccess_execution);
          }
        }
        {    z2zE1989 = (z2zE1988 + zauthorizzation_execution);
        }
      }
      {    zexecution = (z2zE1989 + zcreate_execution);
      }
    }
    uint64_t zinput_length;
    zinput_length = zinput.zlen;
    uint64_t zfloor;
    {
      uint64_t z2zE1982;
      {
        uint64_t z2zE1980;
        {
          uint64_t z2zE1979;
          {
            uint64_t z2zE1978;
            {    z2zE1978 = (zAMSTERDAM_CALLDATA_FLOOR_BYTE * zinput_length);
            }
            {    z2zE1979 = (z2zE1978 + zAMSTERDAM_TX_BASE);
            }
          }
          {    z2zE1980 = (z2zE1979 + zrecipient);
          }
        }
        uint64_t z2zE1981;
        {    z2zE1981 = (zAMSTERDAM_ACCESS_LIST_ADDRESS_FLOOR * zaddress_count);
        }
        {    z2zE1982 = (z2zE1980 + z2zE1981);
        }
      }
      uint64_t z2zE1983;
      {    z2zE1983 = (zAMSTERDAM_ACCESS_LIST_SLOT_FLOOR * zslot_count);
      }
      {    zfloor = (z2zE1982 + z2zE1983);
      }
    }
    struct zIntrinsicGasCost z3zE1821;
    z3zE1821.zcalldata_floor = zfloor;
    z3zE1821.zexecution = zexecution;
    z3zE1821.zstate = UINT64_C(0);
    z8zE453 = z3zE1821;
  }
end_function_2147: ;
  return z8zE453;
end_block_exception_2148: ;
  struct zIntrinsicGasCost z8zE974 = { .zcalldata_floor = UINT64_C(0xdeadc0de), .zexecution = UINT64_C(0xdeadc0de), .zstate = UINT64_C(0xdeadc0de) };
  return z8zE974;
}

uint64_t zmax_blobs_per_transaction(unit z3zE1820)
{
  uint64_t z8zE454;
  bool z2zE1972;
  z2zE1972 = zfork_gteq(zk_fork, zOsaka);
  if (z2zE1972) {  z8zE454 = UINT64_C(6);  } else {
    bool z2zE1973;
    z2zE1973 = zfork_gteq(zk_fork, zPrague);
    if (z2zE1973) {  z8zE454 = UINT64_C(9);  } else {  z8zE454 = UINT64_C(6);  }
  }
end_function_2145: ;
  return z8zE454;
end_block_exception_2146: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t ztransaction_blob_gas_for_count(uint64_t zcount)
{
  uint64_t z8zE455;
  uint64_t zactive_maximum;
  zactive_maximum = zmax_blobs_per_transaction(UNIT);
  bool z2zE1970;
  {
    bool z2zE1969;
    z2zE1969 = (!(UINT64_C(9) < zactive_maximum));
    bool z3zE1819;
    if (z2zE1969) {  z3zE1819 = (!(zactive_maximum < zcount));  } else {  z3zE1819 = false;  }
    z2zE1970 = z3zE1819;
  }
  if (z2zE1970) {
    {    z8zE455 = (UINT64_C(131072) * zcount);
    }
  } else {
    struct zexception z2zE1971;
    CREATE(zexception)(&z2zE1971);
    zInvalidBlock(&z2zE1971, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE1971);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/transaction.sail:275.8-275.44");
    KILL(zexception)(&z2zE1971);
    goto end_block_exception_2144;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1971);
  }
end_function_2143: ;
  return z8zE455;
end_block_exception_2144: ;

  return UINT64_C(0xdeadc0de);
}

struct zTransactionCosts ztransaction_costs(struct zTransaction ztx, uint64_t zgas_limit, sail_u256 zblob_price)
{
  struct zTransactionCosts z8zE456;
  struct zIntrinsicGasCost zintrinsic;
  {
    zintrinsic = zintrinsic_gas(ztx);
    if (have_exception) {  goto end_block_exception_2142;  }
  }
  uint64_t zblob_gas;
  {
    uint64_t z2zE1968;
    {
      struct zBlobHashes z2zE1967;
      z2zE1967 = ztx.zblob_hashes;
      z2zE1968 = z2zE1967.zcount;
    }
    {
      zblob_gas = ztransaction_blob_gas_for_count(z2zE1968);
      if (have_exception) {  goto end_block_exception_2142;  }
    }
  }
  sail_u256 zblob_fee;
  {
    zblob_fee = zblob_word_mulzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zblob_price, zblob_gas);
    if (have_exception) {  goto end_block_exception_2142;  }
  }
  sail_u256 zexecution_cap;
  {
    sail_u256 z2zE1966;
    z2zE1966 = ztx.zmax_fee;
    {
      zexecution_cap = zblob_word_mulzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE1966, zgas_limit);
      if (have_exception) {  goto end_block_exception_2142;  }
    }
  }
  sail_u256 zblob_cap;
  {
    sail_u256 z2zE1965;
    z2zE1965 = ztx.zmax_blob_fee;
    {
      zblob_cap = zblob_word_mulzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE1965, zblob_gas);
      if (have_exception) {  goto end_block_exception_2142;  }
    }
  }
  sail_u256 zupfront;
  {
    sail_u256 z2zE1964;
    {
      sail_u256 z2zE1963;
      z2zE1963 = ztx.zvalue;
      {
        z2zE1964 = zblob_word_add(zexecution_cap, z2zE1963);
        if (have_exception) {  goto end_block_exception_2142;  }
      }
    }
    {
      zupfront = zblob_word_add(z2zE1964, zblob_cap);
      if (have_exception) {  goto end_block_exception_2142;  }
    }
  }
  uint64_t z2zE1960;
  z2zE1960 = zintrinsic.zexecution;
  uint64_t z2zE1961;
  z2zE1961 = zintrinsic.zstate;
  uint64_t z2zE1962;
  z2zE1962 = zintrinsic.zcalldata_floor;
  struct zTransactionCosts z3zE1818;
  z3zE1818.zblob_fee = zblob_fee;
  z3zE1818.zblob_gas = zblob_gas;
  z3zE1818.zcalldata_floor = z2zE1962;
  z3zE1818.zintrinsic_execution = z2zE1960;
  z3zE1818.zintrinsic_state = z2zE1961;
  z3zE1818.zupfront = zupfront;
  z8zE456 = z3zE1818;
end_function_2141: ;
  return z8zE456;
end_block_exception_2142: ;
  struct zTransactionCosts z8zE975 = { .zblob_fee = u256_zero(), .zblob_gas = UINT64_C(0xdeadc0de), .zcalldata_floor = UINT64_C(0xdeadc0de), .zintrinsic_execution = UINT64_C(0xdeadc0de), .zintrinsic_state = UINT64_C(0xdeadc0de), .zupfront = u256_zero() };
  return z8zE975;
}

uint64_t zvalidated_gas_add(uint64_t zleft_gas, uint64_t zright_gas)
{
  uint64_t z8zE457;
  {
    z8zE457 = zconserved_gas_add(zleft_gas, zright_gas);
    if (have_exception) {  goto end_block_exception_2138;  }
  }
end_function_2137: ;
  return z8zE457;
end_block_exception_2138: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zadmitted_transaction_gas_limit(uint64_t zvalue, uint64_t zblock_limit)
{
  uint64_t z8zE458;
  bool z2zE1956;
  z2zE1956 = (zblock_limit < zvalue);
  if (z2zE1956) {
    struct zexception z2zE1957;
    CREATE(zexception)(&z2zE1957);
    zInvalidBlock(&z2zE1957, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE1957);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/transaction.sail:323.8-323.44");
    KILL(zexception)(&z2zE1957);
    goto end_block_exception_2134;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1957);
  } else {  z8zE458 = zvalue;  }
end_function_2133: ;
  return z8zE458;
end_block_exception_2134: ;

  return UINT64_C(0xdeadc0de);
}

__int128 zprocess_auth(struct zAuthorizzation zau)
{
  __int128 z8zE459;
  __int128 zrefund;
  zrefund = zGAS_REFUND_ZERO;
  sail_fixed_bytes_20 zauthority;
  zauthority = zau.zauthority;
  bool z2zE1944;
  {
    bool z2zE1943;
    z2zE1943 = zau.zvalid_sig;
    bool z3zE1806;
    if (z2zE1943) {
      bool z2zE1942;
      {
        sail_u256 z2zE1939;
        z2zE1939 = zau.zchain_id;
        z2zE1942 = zword_is_zzero(z2zE1939);
      }
      bool z3zE1805;
      if (z2zE1942) {  z3zE1805 = true;  } else {
        sail_u256 z2zE1940;
        z2zE1940 = zau.zchain_id;
        uint64_t z2zE1941;
        z2zE1941 = zword_of_chain_identifier(zk_chain_id);
        z3zE1805 = u256_eq_u64(z2zE1940, z2zE1941);
      }
      z3zE1806 = z3zE1805;
    } else {  z3zE1806 = false;  }
    z2zE1944 = z3zE1806;
  }
  unit z3zE1807;
  if (z2zE1944) {
    bool z2zE1945;
    z2zE1945 = zk_access_account(zauthority);
    unit z3zE1808;
    {
      struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1946;
      {
        z2zE1946 = zk_deleg_target(zauthority);
        if (have_exception) {  goto end_block_exception_2132;  }
      }
      unit z3zE1809;
      {
        bool zis_deleg;
        zis_deleg = z2zE1946.ztup0;
        bool z2zE1952;
        {
          bool z2zE1951;
          {
            bool z2zE1948;
            {
              sail_fixed_bytes_32 z2zE1947;
              {
                z2zE1947 = zk_code_key(zauthority);
                if (have_exception) {  goto end_block_exception_2132;  }
              }
              z2zE1948 = eq_fixed_bytes_32(z2zE1947, zKECCAK_EMPTY);
            }
            bool z3zE1810;
            if (z2zE1948) {  z3zE1810 = true;  } else {  z3zE1810 = zis_deleg;  }
            z2zE1951 = z3zE1810;
          }
          bool z3zE1811;
          if (z2zE1951) {
            uint64_t z2zE1949;
            {
              z2zE1949 = zk_get_nonce(zauthority);
              if (have_exception) {  goto end_block_exception_2132;  }
            }
            uint64_t z2zE1950;
            z2zE1950 = zau.znonce;
            z3zE1811 = (z2zE1949 == z2zE1950);
          } else {  z3zE1811 = false;  }
          z2zE1952 = z3zE1811;
        }
        if (z2zE1952) {
          bool zexisted;
          {
            zexisted = zk_account_exists(zauthority);
            if (have_exception) {  goto end_block_exception_2132;  }
          }
          bool z2zE1954;
          {
            sail_fixed_bytes_20 z2zE1953;
            z2zE1953 = zau.zaddress;
            z2zE1954 = eq_fixed_bytes_20(z2zE1953, zZERO_ADDRESS);
          }
          unit z3zE1814;
          if (z2zE1954) {
            {
              z3zE1814 = zk_clear_code(zauthority);
              if (have_exception) {  goto end_block_exception_2132;  }
            }
          } else {
            sail_fixed_bytes_20 z2zE1955;
            z2zE1955 = zau.zaddress;
            {
              z3zE1814 = zk_set_delegation(zauthority, z2zE1955);
              if (have_exception) {  goto end_block_exception_2132;  }
            }
          }
          unit z3zE1813;
          {
            z3zE1813 = zk_bump_nonce(zauthority);
            if (have_exception) {  goto end_block_exception_2132;  }
          }
          if (zexisted) {
            {
              __int128 z3zE1815;
              z3zE1815 = (__int128)(zPER_EMPTY_ACCOUNT);
              __int128 z3zE1816;
              z3zE1816 = (__int128)(zPER_AUTH_BASE);
              zrefund = (z3zE1815 - z3zE1816);
            }
            z3zE1809 = UNIT;
          } else {  z3zE1809 = UNIT;  }
        } else {  z3zE1809 = UNIT;  }
        goto finish_match_2129;
      }
    case_2130: ;
      sail_match_failure("process_auth");
    finish_match_2129: ;
      z3zE1808 = z3zE1809;
      goto finish_match_2127;
    }
  case_2128: ;
    sail_match_failure("process_auth");
  finish_match_2127: ;
    z3zE1807 = z3zE1808;
  } else {  z3zE1807 = UNIT;  }
  z8zE459 = zrefund;
end_function_2131: ;
  return z8zE459;
end_block_exception_2132: ;

  return ((__int128)INT64_C(0xdeadc0de));
}

__int128 zprocess_auth_list(zz5listz8z5structz0zzAuthorizzzzationz9 zxs)
{
  __int128 z8zE460;
  __int128 z3zE1802;
  {
    if (!((zxs == NULL))) goto case_2124;
    z3zE1802 = zGAS_REFUND_ZERO;
    goto finish_match_2122;
  }
case_2124: ;
  {
    /* complete */
    struct zAuthorizzation za;
    za = (*zxs).hd;
    zz5listz8z5structz0zzAuthorizzzzationz9 zr;
    CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&zr);
    COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&zr, (*zxs).tl);
    __int128 z2zE1937;
    {
      z2zE1937 = zprocess_auth(za);
      if (have_exception) {
        KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zr);
        goto end_block_exception_2126;
      }
    }
    __int128 z2zE1938;
    {
      z2zE1938 = zprocess_auth_list(zr);
      if (have_exception) {
        KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zr);
        goto end_block_exception_2126;
      }
    }
    {
      z3zE1802 = zvalidated_refund_add(z2zE1937, z2zE1938);
      if (have_exception) {
        KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zr);
        goto end_block_exception_2126;
      }
    }
    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zr);
    goto finish_match_2122;
  }
case_2123: ;
finish_match_2122: ;
  z8zE460 = z3zE1802;
end_function_2125: ;
  return z8zE460;
end_block_exception_2126: ;

  return ((__int128)INT64_C(0xdeadc0de));
}

bool zauthorizzation_address_seen(sail_fixed_bytes_20 za, zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 zxs)
{
  bool z8zE461;
  bool z3zE1798;
  {
    if (!((zxs == NULL))) goto case_2119;
    z3zE1798 = false;
    goto finish_match_2117;
  }
case_2119: ;
  {
    /* complete */
    sail_fixed_bytes_20 zh;
    zh = (*zxs).hd;
    zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 zt;
    CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zt);
    COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zt, (*zxs).tl);
    bool z2zE1936;
    z2zE1936 = eq_fixed_bytes_20(za, zh);
    bool z3zE1799;
    if (z2zE1936) {  z3zE1799 = true;  } else {  z3zE1799 = zauthorizzation_address_seen(za, zt);  }
    z3zE1798 = z3zE1799;
    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zt);
    goto finish_match_2117;
  }
case_2118: ;
finish_match_2117: ;
  z8zE461 = z3zE1798;
end_function_2120: ;
  return z8zE461;
end_block_exception_2121: ;

  return false;
}

void zprocess_amsterdam_auth(struct zAmsterdamAuthorizzationState *z8zE462, struct zAuthorizzation zau, sail_fixed_bytes_20 zsender, sail_fixed_bytes_20 zcurrent_target, bool ztransfers_value, struct zAmsterdamAuthorizzationState zauth_state)
{
  struct zAmsterdamAuthorizzationState znext;
  CREATE(zAmsterdamAuthorizzationState)(&znext);
  COPY(zAmsterdamAuthorizzationState)(&znext, zauth_state);
  sail_fixed_bytes_20 zauthority;
  zauthority = zau.zauthority;
  bool z2zE1896;
  {
    bool z2zE1895;
    z2zE1895 = zau.zvalid_sig;
    bool z3zE1771;
    if (z2zE1895) {
      bool z2zE1894;
      {
        sail_u256 z2zE1891;
        z2zE1891 = zau.zchain_id;
        z2zE1894 = zword_is_zzero(z2zE1891);
      }
      bool z3zE1770;
      if (z2zE1894) {  z3zE1770 = true;  } else {
        sail_u256 z2zE1892;
        z2zE1892 = zau.zchain_id;
        uint64_t z2zE1893;
        z2zE1893 = zword_of_chain_identifier(zk_chain_id);
        z3zE1770 = u256_eq_u64(z2zE1892, z2zE1893);
      }
      z3zE1771 = z3zE1770;
    } else {  z3zE1771 = false;  }
    z2zE1896 = z3zE1771;
  }
  unit z3zE1772;
  if (z2zE1896) {
    bool z2zE1897;
    z2zE1897 = zk_access_account(zauthority);
    unit z3zE1773;
    {
      struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1898;
      {
        z2zE1898 = zk_deleg_target(zauthority);
        if (have_exception) {
          KILL(zAmsterdamAuthorizzationState)(&znext);
          goto end_block_exception_2116;
        }
      }
      unit z3zE1774;
      {
        bool zcurrently_delegated;
        zcurrently_delegated = z2zE1898.ztup0;
        bool z2zE1904;
        {
          bool z2zE1903;
          {
            bool z2zE1900;
            {
              sail_fixed_bytes_32 z2zE1899;
              {
                z2zE1899 = zk_code_key(zauthority);
                if (have_exception) {
                  KILL(zAmsterdamAuthorizzationState)(&znext);
                  goto end_block_exception_2116;
                }
              }
              z2zE1900 = eq_fixed_bytes_32(z2zE1899, zKECCAK_EMPTY);
            }
            bool z3zE1775;
            if (z2zE1900) {  z3zE1775 = true;  } else {  z3zE1775 = zcurrently_delegated;  }
            z2zE1903 = z3zE1775;
          }
          bool z3zE1776;
          if (z2zE1903) {
            uint64_t z2zE1901;
            {
              z2zE1901 = zk_get_nonce(zauthority);
              if (have_exception) {
                KILL(zAmsterdamAuthorizzationState)(&znext);
                goto end_block_exception_2116;
              }
            }
            uint64_t z2zE1902;
            z2zE1902 = zau.znonce;
            z3zE1776 = (z2zE1901 == z2zE1902);
          } else {  z3zE1776 = false;  }
          z2zE1904 = z3zE1776;
        }
        if (z2zE1904) {
          bool zseen;
          {
            zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1935;
            CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1935);
            COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1935, zauth_state.zseen_valid_authorities);
            zseen = zauthorizzation_address_seen(zauthority, z2zE1935);
            KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1935);
          }
          bool zdelegated_before_tx;
          if (zseen) {
            zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1934;
            CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1934);
            COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1934, zauth_state.zoriginally_delegated);
            zdelegated_before_tx = zauthorizzation_address_seen(zauthority, z2zE1934);
            KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1934);
          } else {  zdelegated_before_tx = zcurrently_delegated;  }
          bool zalready_written;
          {
            bool z3zE1780;
            if (zseen) {  z3zE1780 = true;  } else {
              bool z2zE1933;
              z2zE1933 = eq_fixed_bytes_20(zauthority, zsender);
              bool z3zE1779;
              if (z2zE1933) {  z3zE1779 = true;  } else {
                bool z3zE1778;
                if (ztransfers_value) {  z3zE1778 = eq_fixed_bytes_20(zauthority, zcurrent_target);  } else {
                  z3zE1778 = false;
                }
                z3zE1779 = z3zE1778;
              }
              z3zE1780 = z3zE1779;
            }
            zalready_written = z3zE1780;
          }
          bool z2zE1906;
          {
            bool z2zE1905;
            {
              z2zE1905 = zk_account_exists(zauthority);
              if (have_exception) {
                KILL(zAmsterdamAuthorizzationState)(&znext);
                goto end_block_exception_2116;
              }
            }
            z2zE1906 = not(z2zE1905);
          }
          unit z3zE1787;
          if (z2zE1906) {
            {
              z3zE1787 = zcharge_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_amsterdam_state_new_account);
              if (have_exception) {
                KILL(zAmsterdamAuthorizzationState)(&znext);
                goto end_block_exception_2116;
              }
            }
          } else {  z3zE1787 = UNIT;  }
          bool z2zE1908;
          {
            bool z2zE1907;
            z2zE1907 = zis_running(UNIT);
            bool z3zE1781;
            if (z2zE1907) {  z3zE1781 = not(zalready_written);  } else {  z3zE1781 = false;  }
            z2zE1908 = z3zE1781;
          }
          unit z3zE1786;
          if (z2zE1908) {
            {
              z3zE1786 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_amsterdam_account_write);
              if (have_exception) {
                KILL(zAmsterdamAuthorizzationState)(&znext);
                goto end_block_exception_2116;
              }
            }
          } else {  z3zE1786 = UNIT;  }
          bool z2zE1915;
          {
            bool z2zE1914;
            z2zE1914 = zis_running(UNIT);
            bool z3zE1784;
            if (z2zE1914) {
              bool z2zE1913;
              {
                sail_fixed_bytes_20 z2zE1909;
                z2zE1909 = zau.zaddress;
                z2zE1913 = zneq_anythingzIR__sail_c_repr_fixed_byteszIC20zKzK(z2zE1909, zZERO_ADDRESS);
              }
              bool z3zE1783;
              if (z2zE1913) {
                bool z2zE1912;
                z2zE1912 = not(zdelegated_before_tx);
                bool z3zE1782;
                if (z2zE1912) {
                  bool z2zE1911;
                  {
                    zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1910;
                    CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1910);
                    COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1910, zauth_state.zdelegation_set_for);
                    z2zE1911 = zauthorizzation_address_seen(zauthority, z2zE1910);
                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1910);
                  }
                  z3zE1782 = not(z2zE1911);
                } else {  z3zE1782 = false;  }
                z3zE1783 = z3zE1782;
              } else {  z3zE1783 = false;  }
              z3zE1784 = z3zE1783;
            } else {  z3zE1784 = false;  }
            z2zE1915 = z3zE1784;
          }
          unit z3zE1785;
          if (z2zE1915) {
            {
              z3zE1785 = zcharge_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_amsterdam_state_auth_base);
              if (have_exception) {
                KILL(zAmsterdamAuthorizzationState)(&znext);
                goto end_block_exception_2116;
              }
            }
          } else {  z3zE1785 = UNIT;  }
          bool z2zE1916;
          z2zE1916 = zis_running(UNIT);
          if (z2zE1916) {
            bool z2zE1918;
            {
              sail_fixed_bytes_20 z2zE1917;
              z2zE1917 = zau.zaddress;
              z2zE1918 = eq_fixed_bytes_20(z2zE1917, zZERO_ADDRESS);
            }
            unit z3zE1794;
            if (z2zE1918) {
              {
                z3zE1794 = zk_clear_code(zauthority);
                if (have_exception) {
                  KILL(zAmsterdamAuthorizzationState)(&znext);
                  goto end_block_exception_2116;
                }
              }
            } else {
              sail_fixed_bytes_20 z2zE1919;
              z2zE1919 = zau.zaddress;
              {
                z3zE1794 = zk_set_delegation(zauthority, z2zE1919);
                if (have_exception) {
                  KILL(zAmsterdamAuthorizzationState)(&znext);
                  goto end_block_exception_2116;
                }
              }
            }
            unit z3zE1793;
            {
              z3zE1793 = zk_bump_nonce(zauthority);
              if (have_exception) {
                KILL(zAmsterdamAuthorizzationState)(&znext);
                goto end_block_exception_2116;
              }
            }
            zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1921;
            CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1921);
            {
              zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1920;
              CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1920);
              COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1920, znext.zseen_valid_authorities);
              zconsz3z5structz0zz__sail_c_repr_fixed_bytesz820z9(&z2zE1921, zauthority, z2zE1920);
              KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1920);
            }
            COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&znext)->zseen_valid_authorities), z2zE1921);
            unit z3zE1792;
            z3zE1792 = UNIT;
            bool z2zE1923;
            {
              bool z2zE1922;
              z2zE1922 = not(zseen);
              bool z3zE1789;
              if (z2zE1922) {  z3zE1789 = zcurrently_delegated;  } else {  z3zE1789 = false;  }
              z2zE1923 = z3zE1789;
            }
            unit z3zE1790;
            if (z2zE1923) {
              zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1925;
              CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1925);
              {
                zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1924;
                CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1924);
                COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1924, znext.zoriginally_delegated);
                zconsz3z5structz0zz__sail_c_repr_fixed_bytesz820z9(&z2zE1925, zauthority, z2zE1924);
              }
              struct zAmsterdamAuthorizzationState z3zE1791;
              CREATE(zAmsterdamAuthorizzationState)(&z3zE1791);
              COPY(zAmsterdamAuthorizzationState)(&z3zE1791, znext);
              COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE1791)->zoriginally_delegated), z2zE1925);
              COPY(zAmsterdamAuthorizzationState)(&znext, z3zE1791);
              z3zE1790 = UNIT;
            } else {  z3zE1790 = UNIT;  }
            bool z2zE1930;
            {
              bool z2zE1929;
              {
                sail_fixed_bytes_20 z2zE1926;
                z2zE1926 = zau.zaddress;
                z2zE1929 = zneq_anythingzIR__sail_c_repr_fixed_byteszIC20zKzK(z2zE1926, zZERO_ADDRESS);
              }
              bool z3zE1795;
              if (z2zE1929) {
                bool z2zE1928;
                {
                  zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1927;
                  CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1927);
                  COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1927, znext.zdelegation_set_for);
                  z2zE1928 = zauthorizzation_address_seen(zauthority, z2zE1927);
                }
                z3zE1795 = not(z2zE1928);
              } else {  z3zE1795 = false;  }
              z2zE1930 = z3zE1795;
            }
            if (z2zE1930) {
              zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1932;
              CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1932);
              {
                zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1931;
                CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1931);
                COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1931, znext.zdelegation_set_for);
                zconsz3z5structz0zz__sail_c_repr_fixed_bytesz820z9(&z2zE1932, zauthority, z2zE1931);
              }
              struct zAmsterdamAuthorizzationState z3zE1796;
              CREATE(zAmsterdamAuthorizzationState)(&z3zE1796);
              COPY(zAmsterdamAuthorizzationState)(&z3zE1796, znext);
              COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE1796)->zdelegation_set_for), z2zE1932);
              COPY(zAmsterdamAuthorizzationState)(&znext, z3zE1796);
              z3zE1774 = UNIT;
            } else {  z3zE1774 = UNIT;  }
          } else {  z3zE1774 = UNIT;  }
        } else {  z3zE1774 = UNIT;  }
        goto finish_match_2113;
      }
    case_2114: ;
      sail_match_failure("process_amsterdam_auth");
    finish_match_2113: ;
      z3zE1773 = z3zE1774;
      goto finish_match_2111;
    }
  case_2112: ;
    sail_match_failure("process_amsterdam_auth");
  finish_match_2111: ;
    z3zE1772 = z3zE1773;
  } else {  z3zE1772 = UNIT;  }
  COPY(zAmsterdamAuthorizzationState)((*(&z8zE462)), znext);
  KILL(zAmsterdamAuthorizzationState)(&znext);
end_function_2115: ;
  goto end_function_4045;
end_block_exception_2116: ;
  goto end_function_4045;
end_function_4045: ;
}

void zprocess_amsterdam_auth_list(struct zAmsterdamAuthorizzationState *z8zE463, zz5listz8z5structz0zzAuthorizzzzationz9 zxs, sail_fixed_bytes_20 zsender, sail_fixed_bytes_20 zcurrent_target, bool ztransfers_value, struct zAmsterdamAuthorizzationState zauth_state)
{
  struct zAmsterdamAuthorizzationState z3zE1767;
  CREATE(zAmsterdamAuthorizzationState)(&z3zE1767);
  {
    if (!((zxs == NULL))) goto case_2108;
    COPY(zAmsterdamAuthorizzationState)(&z3zE1767, zauth_state);
    goto finish_match_2106;
  }
case_2108: ;
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
        KILL(zAmsterdamAuthorizzationState)(&z3zE1767);
        KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zr);
        KILL(zAmsterdamAuthorizzationState)(&znext);
        goto end_block_exception_2110;
      }
    }
    bool z2zE1890;
    z2zE1890 = zis_running(UNIT);
    if (z2zE1890) {
      {
        zprocess_amsterdam_auth_list(&z3zE1767, zr, zsender, zcurrent_target, ztransfers_value, znext);
        if (have_exception) {
          KILL(zAmsterdamAuthorizzationState)(&z3zE1767);
          KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zr);
          KILL(zAmsterdamAuthorizzationState)(&znext);
          goto end_block_exception_2110;
        }
      }
    } else {  COPY(zAmsterdamAuthorizzationState)(&z3zE1767, znext);  }
    KILL(zAmsterdamAuthorizzationState)(&znext);
    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zr);
    goto finish_match_2106;
  }
case_2107: ;
finish_match_2106: ;
  COPY(zAmsterdamAuthorizzationState)((*(&z8zE463)), z3zE1767);
  KILL(zAmsterdamAuthorizzationState)(&z3zE1767);
end_function_2109: ;
  goto end_function_4044;
end_block_exception_2110: ;
  goto end_function_4044;
end_function_4044: ;
}

unit zwarm_access_list_addresses(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 zxs)
{
  unit z8zE464;
  unit z3zE1762;
  {
    if (!((zxs == NULL))) goto case_2103;
    z3zE1762 = UNIT;
    goto finish_match_2099;
  }
case_2103: ;
  {
    /* complete */
    sail_fixed_bytes_20 za;
    za = (*zxs).hd;
    zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 zr;
    CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zr);
    COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zr, (*zxs).tl);
    bool z2zE1889;
    z2zE1889 = zk_access_account(za);
    unit z3zE1763;
    {
      z3zE1763 = zwarm_access_list_addresses(zr);
      goto finish_match_2101;
    }
  case_2102: ;
    sail_match_failure("warm_access_list_addresses");
  finish_match_2101: ;
    z3zE1762 = z3zE1763;
    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zr);
    goto finish_match_2099;
  }
case_2100: ;
finish_match_2099: ;
  z8zE464 = z3zE1762;
end_function_2104: ;
  return z8zE464;
end_block_exception_2105: ;

  return UNIT;
}

unit zwarm_access_list_slots(zz5listz8z5structz0zzStorageKeyz9 zxs)
{
  unit z8zE465;
  unit z3zE1757;
  {
    if (!((zxs == NULL))) goto case_2096;
    z3zE1757 = UNIT;
    goto finish_match_2092;
  }
case_2096: ;
  {
    /* complete */
    struct zStorageKey zk;
    zk = (*zxs).hd;
    zz5listz8z5structz0zzStorageKeyz9 zr;
    CREATE(zz5listz8z5structz0zzStorageKeyz9)(&zr);
    COPY(zz5listz8z5structz0zzStorageKeyz9)(&zr, (*zxs).tl);
    bool z2zE1888;
    {
      sail_fixed_bytes_20 z2zE1886;
      z2zE1886 = zk.zaddr;
      sail_u256 z2zE1887;
      z2zE1887 = zk.zslot;
      z2zE1888 = zk_slot_is_warm(z2zE1886, z2zE1887);
    }
    unit z3zE1758;
    {
      z3zE1758 = zwarm_access_list_slots(zr);
      goto finish_match_2094;
    }
  case_2095: ;
    sail_match_failure("warm_access_list_slots");
  finish_match_2094: ;
    z3zE1757 = z3zE1758;
    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zr);
    goto finish_match_2092;
  }
case_2093: ;
finish_match_2092: ;
  z8zE465 = z3zE1757;
end_function_2097: ;
  return z8zE465;
end_block_exception_2098: ;

  return UNIT;
}

unit zprewarm(struct zTransaction ztx)
{
  unit z8zE466;
  bool z2zE1872;
  {
    sail_fixed_bytes_20 z2zE1871;
    z2zE1871 = ztx.zsender;
    z2zE1872 = zk_access_account(z2zE1871);
  }
  unit z3zE1737;
  {
    bool z2zE1875;
    {
      bool z2zE1873;
      z2zE1873 = ztx.zis_create;
      if (z2zE1873) {  z2zE1875 = false;  } else {
        sail_fixed_bytes_20 z2zE1874;
        z2zE1874 = ztx.zrecipient;
        z2zE1875 = zk_access_account(z2zE1874);
      }
    }
    unit z3zE1738;
    {
      bool z2zE1876;
      z2zE1876 = zfork_gteq(zk_fork, zShanghai);
      unit z3zE1739;
      if (z2zE1876) {
        bool z2zE1878;
        {
          sail_fixed_bytes_20 z2zE1877;
          z2zE1877 = zk_coinbase(UNIT);
          z2zE1878 = zk_access_account(z2zE1877);
        }
        unit z3zE1740;
        {
          z3zE1740 = UNIT;
          goto finish_match_2082;
        }
      case_2083: ;
        sail_match_failure("prewarm");
      finish_match_2082: ;
        z3zE1739 = z3zE1740;
      } else {  z3zE1739 = UNIT;  }
      zz5vecz8z5u64z9 zprecompile_addresses;
      CREATE(zz5vecz8z5u64z9)(&zprecompile_addresses);
      {
        zz5vecz8z5u64z9 z3zE1742;
        CREATE(zz5vecz8z5u64z9)(&z3zE1742);
        internal_vector_init_zz5vecz8z5u64z9(&z3zE1742, INT64_C(17));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1742, z3zE1742, INT64_C(0), UINT64_C(17));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1742, z3zE1742, INT64_C(1), UINT64_C(16));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1742, z3zE1742, INT64_C(2), UINT64_C(15));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1742, z3zE1742, INT64_C(3), UINT64_C(14));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1742, z3zE1742, INT64_C(4), UINT64_C(13));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1742, z3zE1742, INT64_C(5), UINT64_C(12));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1742, z3zE1742, INT64_C(6), UINT64_C(11));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1742, z3zE1742, INT64_C(7), UINT64_C(10));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1742, z3zE1742, INT64_C(8), UINT64_C(9));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1742, z3zE1742, INT64_C(9), UINT64_C(8));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1742, z3zE1742, INT64_C(10), UINT64_C(7));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1742, z3zE1742, INT64_C(11), UINT64_C(6));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1742, z3zE1742, INT64_C(12), UINT64_C(5));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1742, z3zE1742, INT64_C(13), UINT64_C(4));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1742, z3zE1742, INT64_C(14), UINT64_C(3));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1742, z3zE1742, INT64_C(15), UINT64_C(2));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1742, z3zE1742, INT64_C(16), UINT64_C(1));
        KILL(zz5vecz8z5u64z9)(&zprecompile_addresses);
        internal_vector_init_zz5vecz8z5u64z9(&zprecompile_addresses, z3zE1742.len);
        for (int z8zE976 = 0; z8zE976 < z3zE1742.len; z8zE976++) {
          uint64_t z8zE977 = z3zE1742.data[z8zE976];
          uint64_t z8zE978;
          z8zE978 = z8zE977;
          zprecompile_addresses.data[z8zE976] = z8zE978;
        }
        KILL(zz5vecz8z5u64z9)(&z3zE1742);
      }
      int64_t z3zE1743;
      {    z3zE1743 = (int64_t)(UINT64_C(0));
      }
      int64_t z3zE1744;
      {    z3zE1744 = (int64_t)(UINT64_C(16));
      }
      int64_t z3zE1745;
      {    z3zE1745 = (int64_t)(UINT64_C(1));
      }
      {
        int64_t zi;
        zi = z3zE1743;
        unit z3zE1746;
      for_start_2084: ;
        {
          if ((z3zE1744 < zi)) goto for_end_2085;
          uint64_t zp;
          zp = fast_vector_access_zz5vecz8z5u64z9(zprecompile_addresses, zi);
          bool z2zE1879;
          z2zE1879 = zprecompile_active_at_fork(zp);
          if (z2zE1879) {
            bool z2zE1881;
            {
              sail_fixed_bytes_20 z2zE1880;
              z2zE1880 = zprecompile_id_to_address(zp);
              z2zE1881 = zk_access_account(z2zE1880);
            }
            unit z3zE1747;
            {
              z3zE1747 = UNIT;
              goto finish_match_2086;
            }
          case_2087: ;
            sail_match_failure("prewarm");
          finish_match_2086: ;
            z3zE1746 = z3zE1747;
          } else {  z3zE1746 = UNIT;  }
          zi = (zi + z3zE1745);
          goto for_start_2084;
        }
      for_end_2085: ;
      }
      unit z3zE1754;
      z3zE1754 = UNIT;
      bool z2zE1882;
      z2zE1882 = zprecompile_active_at_fork(UINT64_C(256));
      unit z3zE1750;
      if (z2zE1882) {
        bool z2zE1883;
        {
          zz5vecz8z5bv8z9 z3zE1751;
          CREATE(zz5vecz8z5bv8z9)(&z3zE1751);
          internal_vector_init_zz5vecz8z5bv8z9(&z3zE1751, INT64_C(20));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1751, z3zE1751, INT64_C(0), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1751, z3zE1751, INT64_C(1), UINT64_C(0x01));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1751, z3zE1751, INT64_C(2), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1751, z3zE1751, INT64_C(3), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1751, z3zE1751, INT64_C(4), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1751, z3zE1751, INT64_C(5), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1751, z3zE1751, INT64_C(6), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1751, z3zE1751, INT64_C(7), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1751, z3zE1751, INT64_C(8), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1751, z3zE1751, INT64_C(9), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1751, z3zE1751, INT64_C(10), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1751, z3zE1751, INT64_C(11), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1751, z3zE1751, INT64_C(12), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1751, z3zE1751, INT64_C(13), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1751, z3zE1751, INT64_C(14), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1751, z3zE1751, INT64_C(15), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1751, z3zE1751, INT64_C(16), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1751, z3zE1751, INT64_C(17), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1751, z3zE1751, INT64_C(18), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1751, z3zE1751, INT64_C(19), UINT64_C(0x00));
          {
            sail_fixed_bytes_20 z3zE3795;
            for (size_t z8zE979 = 0; z8zE979 < 20; ++z8zE979) {
              z3zE3795.bytes[z8zE979] = (uint8_t)(z3zE1751.data[z8zE979] & UINT64_C(0xff));
            }
            z2zE1883 = zk_access_account(z3zE3795);
          }
          KILL(zz5vecz8z5bv8z9)(&z3zE1751);
        }
        unit z3zE1752;
        {
          z3zE1752 = UNIT;
          goto finish_match_2088;
        }
      case_2089: ;
        sail_match_failure("prewarm");
      finish_match_2088: ;
        z3zE1750 = z3zE1752;
      } else {  z3zE1750 = UNIT;  }
      zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1884;
      CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1884);
      COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1884, ztx.zaccess_list_addresses);
      unit z3zE1749;
      z3zE1749 = zwarm_access_list_addresses(z2zE1884);
      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1884);
      zz5listz8z5structz0zzStorageKeyz9 z2zE1885;
      CREATE(zz5listz8z5structz0zzStorageKeyz9)(&z2zE1885);
      COPY(zz5listz8z5structz0zzStorageKeyz9)(&z2zE1885, ztx.zaccess_list_slots);
      z3zE1738 = zwarm_access_list_slots(z2zE1885);
      KILL(zz5listz8z5structz0zzStorageKeyz9)(&z2zE1885);
      KILL(zz5vecz8z5u64z9)(&zprecompile_addresses);
      goto finish_match_2080;
    }
  case_2081: ;
    sail_match_failure("prewarm");
  finish_match_2080: ;
    z3zE1737 = z3zE1738;
    goto finish_match_2078;
  }
case_2079: ;
  sail_match_failure("prewarm");
finish_match_2078: ;
  z8zE466 = z3zE1737;
end_function_2090: ;
  return z8zE466;
end_block_exception_2091: ;

  return UNIT;
}

struct ztuple_z8z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256z9 zeff_gas_price_for(sail_u256 zbase_fee, sail_u256 zmax_fee, sail_u256 zmax_priority_fee)
{
  struct ztuple_z8z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256z9 z8zE467;
  sail_u256 zprice;
  {
    bool z2zE1869;
    z2zE1869 = zword_ule(zmax_fee, zbase_fee);
    if (z2zE1869) {  zprice = zmax_fee;  } else {
      sail_u256 zavailable_priority;
      zavailable_priority = zword_sub_word(zmax_fee, zbase_fee);
      bool z2zE1870;
      z2zE1870 = zword_ule(zmax_priority_fee, zavailable_priority);
      if (z2zE1870) {  zprice = zword_add_word(zbase_fee, zmax_priority_fee);  } else {  zprice = zmax_fee;  }
    }
  }
  sail_u256 zpriority;
  {
    bool z2zE1868;
    z2zE1868 = zword_ule(zbase_fee, zprice);
    if (z2zE1868) {  zpriority = zword_sub_word(zprice, zbase_fee);  } else {  zpriority = zZERO_WORD;  }
  }
  struct ztuple_z8z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256z9 z3zE1736;
  z3zE1736.ztup0 = zprice;
  z3zE1736.ztup1 = zpriority;
  z8zE467 = z3zE1736;
end_function_2076: ;
  return z8zE467;
end_block_exception_2077: ;
  struct ztuple_z8z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256z9 z8zE980 = { .ztup0 = u256_zero(), .ztup1 = u256_zero() };
  return z8zE980;
}

struct zTxValidity zcheck_transaction_validity(struct zTransaction ztx)
{
  struct zTxValidity z8zE468;
  bool z2zE1754;
  {
    bool z2zE1753;
    {
      enum zTxType z2zE1751;
      z2zE1751 = ztx.ztx_type;
      sail_u256 z2zE1752;
      z2zE1752 = ztx.zsig_v;
      z2zE1753 = ztx_sig_v_valid(zk_chain_id, z2zE1751, z2zE1752);
    }
    z2zE1754 = not(z2zE1753);
  }
  unit z3zE1689;
  if (z2zE1754) {
    struct zexception z2zE1755;
    CREATE(zexception)(&z2zE1755);
    zInvalidBlock(&z2zE1755, zInvalidSignature);
    COPY(zexception)(current_exception, z2zE1755);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/transaction.sail:558.8-558.44");
    KILL(zexception)(&z2zE1755);
    goto end_block_exception_2075;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1755);
  } else {  z3zE1689 = UNIT;  }
  uint64_t zparity;
  {
    enum zTxType z2zE1866;
    z2zE1866 = ztx.ztx_type;
    sail_u256 z2zE1867;
    z2zE1867 = ztx.zsig_v;
    zparity = ztx_y_parity(z2zE1866, z2zE1867);
  }
  bool z2zE1761;
  {
    bool z2zE1760;
    {
      sail_fixed_bytes_20 z2zE1756;
      z2zE1756 = ztx.zsender;
      sail_fixed_bytes_32 z2zE1757;
      z2zE1757 = ztx.zsigning_hash;
      sail_u256 z2zE1758;
      z2zE1758 = ztx.zsig_r;
      sail_u256 z2zE1759;
      z2zE1759 = ztx.zsig_s;
      z2zE1760 = ztx_auth_valid(z2zE1756, z2zE1757, zparity, z2zE1758, z2zE1759);
    }
    z2zE1761 = not(z2zE1760);
  }
  unit z3zE1690;
  if (z2zE1761) {
    struct zexception z2zE1762;
    CREATE(zexception)(&z2zE1762);
    zInvalidBlock(&z2zE1762, zInvalidSignature);
    COPY(zexception)(current_exception, z2zE1762);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/transaction.sail:562.8-562.44");
    KILL(zexception)(&z2zE1762);
    goto end_block_exception_2075;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1762);
  } else {  z3zE1690 = UNIT;  }
  uint64_t zgas_limit;
  {
    uint64_t z2zE1864;
    z2zE1864 = ztx.zgas_limit;
    uint64_t z2zE1865;
    z2zE1865 = zk_header.zgas_limit;
    {
      zgas_limit = zadmitted_transaction_gas_limit(z2zE1864, z2zE1865);
      if (have_exception) {  goto end_block_exception_2075;  }
    }
  }
  struct ztuple_z8z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256z9 z2zE1766;
  {
    sail_u256 z2zE1763;
    z2zE1763 = zk_header.zbase_fee;
    sail_u256 z2zE1764;
    z2zE1764 = ztx.zmax_fee;
    sail_u256 z2zE1765;
    z2zE1765 = ztx.zmax_priority_fee;
    z2zE1766 = zeff_gas_price_for(z2zE1763, z2zE1764, z2zE1765);
  }
  struct zTxValidity z3zE1691;
  {
    sail_u256 zeff_gas_price;
    zeff_gas_price = z2zE1766.ztup0;
    sail_u256 zeff_priority_fee;
    zeff_priority_fee = z2zE1766.ztup1;
    sail_fixed_bytes_20 zsender;
    zsender = ztx.zsender;
    struct zByteSliceFields zinput;
    zinput = ztx.zinput_src;
    uint64_t zinput_len;
    zinput_len = zinput.zlen;
    uint64_t znonce_before;
    {
      znonce_before = zk_get_nonce(zsender);
      if (have_exception) {  goto end_block_exception_2075;  }
    }
    sail_u256 zblob_price;
    {
      uint64_t z2zE1863;
      z2zE1863 = zk_header.zexcess_blob_gas;
      {
        zblob_price = zblob_base_fee(z2zE1863);
        if (have_exception) {  goto end_block_exception_2075;  }
      }
    }
    struct zTransactionCosts zcosts;
    {
      zcosts = ztransaction_costs(ztx, zgas_limit, zblob_price);
      if (have_exception) {  goto end_block_exception_2075;  }
    }
    uint64_t znonce;
    {
      struct zoptionzIU64zK z2zE1861;
      CREATE(zoptionzIU64zK)(&z2zE1861);
      {
        sail_u256 z2zE1860;
        z2zE1860 = ztx.znonce;
        zword_to_account_nonce(&z2zE1861, z2zE1860);
      }
      uint64_t z3zE1692;
      {
        if (z2zE1861.kind != Kind_zSomezIU64zK) goto case_2068;
        uint64_t z1zE1;
        z1zE1 = z2zE1861.variants.zSomezIU64zK;
        z3zE1692 = z1zE1;
        goto finish_match_2066;
      }
    case_2068: ;
      {
        /* complete */
        struct zexception z2zE1862;
        CREATE(zexception)(&z2zE1862);
        zInvalidBlock(&z2zE1862, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1862);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:576.18-576.54");
        KILL(zoptionzIU64zK)(&z2zE1861);
        KILL(zexception)(&z2zE1862);
        goto end_block_exception_2075;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1862);
        goto finish_match_2066;
      }
    case_2067: ;
    finish_match_2066: ;
      znonce = z3zE1692;
      KILL(zoptionzIU64zK)(&z2zE1861);
    }
    bool z2zE1767;
    z2zE1767 = (znonce != znonce_before);
    unit z3zE1695;
    if (z2zE1767) {
      struct zexception z2zE1768;
      CREATE(zexception)(&z2zE1768);
      zInvalidBlock(&z2zE1768, zExecutionInvalid);
      COPY(zexception)(current_exception, z2zE1768);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/evm/transaction.sail:579.8-579.44");
      KILL(zexception)(&z2zE1768);
      goto end_block_exception_2075;
      /* unreachable after throw */
      KILL(zexception)(&z2zE1768);
    } else {  z3zE1695 = UNIT;  }
    struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1769;
    {
      z2zE1769 = zk_deleg_target(zsender);
      if (have_exception) {  goto end_block_exception_2075;  }
    }
    struct zTxValidity z3zE1696;
    {
      bool zsender_deleg;
      zsender_deleg = z2zE1769.ztup0;
      uint64_t zmax_blobs;
      zmax_blobs = zmax_blobs_per_transaction(UNIT);
      bool z2zE1771;
      {
        enum zTxType z2zE1770;
        z2zE1770 = ztx.ztx_type;
        z2zE1771 = ztx_is_blob(z2zE1770);
      }
      unit z3zE1702;
      if (z2zE1771) {
        bool z2zE1779;
        {
          bool z2zE1778;
          z2zE1778 = zfork_lt(zk_fork, zCancun);
          bool z3zE1705;
          if (z2zE1778) {  z3zE1705 = true;  } else {
            bool z2zE1777;
            {
              uint64_t z2zE1773;
              {
                struct zBlobHashes z2zE1772;
                z2zE1772 = ztx.zblob_hashes;
                z2zE1773 = z2zE1772.zcount;
              }
              z2zE1777 = (z2zE1773 == UINT64_C(0));
            }
            bool z3zE1704;
            if (z2zE1777) {  z3zE1704 = true;  } else {
              bool z2zE1776;
              {
                uint64_t z2zE1775;
                {
                  struct zBlobHashes z2zE1774;
                  z2zE1774 = ztx.zblob_hashes;
                  z2zE1775 = z2zE1774.zcount;
                }
                z2zE1776 = (zmax_blobs < z2zE1775);
              }
              bool z3zE1703;
              if (z2zE1776) {  z3zE1703 = true;  } else {  z3zE1703 = ztx.zis_create;  }
              z3zE1704 = z3zE1703;
            }
            z3zE1705 = z3zE1704;
          }
          z2zE1779 = z3zE1705;
        }
        unit z3zE1706;
        if (z2zE1779) {
          struct zexception z2zE1780;
          CREATE(zexception)(&z2zE1780);
          zInvalidBlock(&z2zE1780, zExecutionInvalid);
          COPY(zexception)(current_exception, z2zE1780);
          have_exception = true;
          COPY(sail_string)(throw_location, "sail/evm/transaction.sail:593.12-593.48");
          KILL(zexception)(&z2zE1780);
          goto end_block_exception_2075;
          /* unreachable after throw */
          KILL(zexception)(&z2zE1780);
        } else {  z3zE1706 = UNIT;  }
        struct zBlobHashes z2zE1781;
        z2zE1781 = ztx.zblob_hashes;
        {
          z3zE1702 = zvalidate_blob_hash_versions(z2zE1781);
          if (have_exception) {  goto end_block_exception_2075;  }
        }
      } else {  z3zE1702 = UNIT;  }
      bool z2zE1784;
      {
        bool z2zE1783;
        z2zE1783 = zfork_gteq(zk_fork, zPrague);
        bool z3zE1697;
        if (z2zE1783) {
          uint64_t z2zE1782;
          z2zE1782 = zcosts.zcalldata_floor;
          z3zE1697 = (zgas_limit < z2zE1782);
        } else {  z3zE1697 = false;  }
        z2zE1784 = z3zE1697;
      }
      unit z3zE1701;
      if (z2zE1784) {
        struct zexception z2zE1785;
        CREATE(zexception)(&z2zE1785);
        zInvalidBlock(&z2zE1785, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1785);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:601.8-601.44");
        KILL(zexception)(&z2zE1785);
        goto end_block_exception_2075;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1785);
      } else {  z3zE1701 = UNIT;  }
      bool z2zE1789;
      {
        bool z2zE1788;
        {
          sail_u256 z2zE1786;
          z2zE1786 = zcosts.zupfront;
          sail_u256 z2zE1787;
          {
            z2zE1787 = zk_get_balance(zsender);
            if (have_exception) {  goto end_block_exception_2075;  }
          }
          z2zE1788 = zword_ule(z2zE1786, z2zE1787);
        }
        z2zE1789 = not(z2zE1788);
      }
      unit z3zE1700;
      if (z2zE1789) {
        struct zexception z2zE1790;
        CREATE(zexception)(&z2zE1790);
        zInvalidBlock(&z2zE1790, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1790);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:604.8-604.44");
        KILL(zexception)(&z2zE1790);
        goto end_block_exception_2075;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1790);
      } else {  z3zE1700 = UNIT;  }
      bool z2zE1794;
      {
        bool z2zE1793;
        {
          bool z2zE1792;
          {
            sail_fixed_bytes_32 z2zE1791;
            {
              z2zE1791 = zk_code_key(zsender);
              if (have_exception) {  goto end_block_exception_2075;  }
            }
            z2zE1792 = eq_fixed_bytes_32(z2zE1791, zKECCAK_EMPTY);
          }
          bool z3zE1698;
          if (z2zE1792) {  z3zE1698 = true;  } else {  z3zE1698 = zsender_deleg;  }
          z2zE1793 = z3zE1698;
        }
        z2zE1794 = not(z2zE1793);
      }
      unit z3zE1699;
      if (z2zE1794) {
        struct zexception z2zE1795;
        CREATE(zexception)(&z2zE1795);
        zInvalidBlock(&z2zE1795, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1795);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:607.8-607.44");
        KILL(zexception)(&z2zE1795);
        goto end_block_exception_2075;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1795);
      } else {  z3zE1699 = UNIT;  }
      sail_u128 zintrinsic_total;
      {
        uint64_t z2zE1858;
        z2zE1858 = zcosts.zintrinsic_execution;
        uint64_t z2zE1859;
        z2zE1859 = zcosts.zintrinsic_state;
        zintrinsic_total = u128_add_u64_u64(z2zE1858, z2zE1859);
      }
      bool z2zE1796;
      z2zE1796 = u64_lt_u128(zgas_limit, zintrinsic_total);
      unit z3zE1732;
      if (z2zE1796) {
        struct zexception z2zE1797;
        CREATE(zexception)(&z2zE1797);
        zInvalidBlock(&z2zE1797, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1797);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:611.8-611.44");
        KILL(zexception)(&z2zE1797);
        goto end_block_exception_2075;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1797);
      } else {  z3zE1732 = UNIT;  }
      bool z2zE1800;
      {
        bool z2zE1799;
        z2zE1799 = zfork_gteq(zk_fork, zAmsterdam);
        bool z3zE1707;
        if (z2zE1799) {
          uint64_t z2zE1798;
          z2zE1798 = zcosts.zintrinsic_execution;
          z3zE1707 = (zAMSTERDAM_TX_MAX_GAS < z2zE1798);
        } else {  z3zE1707 = false;  }
        z2zE1800 = z3zE1707;
      }
      unit z3zE1731;
      if (z2zE1800) {
        struct zexception z2zE1801;
        CREATE(zexception)(&z2zE1801);
        zInvalidBlock(&z2zE1801, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1801);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:614.8-614.44");
        KILL(zexception)(&z2zE1801);
        goto end_block_exception_2075;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1801);
      } else {  z3zE1731 = UNIT;  }
      bool z2zE1804;
      {
        bool z2zE1803;
        z2zE1803 = zfork_gteq(zk_fork, zAmsterdam);
        bool z3zE1708;
        if (z2zE1803) {
          uint64_t z2zE1802;
          z2zE1802 = zcosts.zcalldata_floor;
          z3zE1708 = (zAMSTERDAM_TX_MAX_GAS < z2zE1802);
        } else {  z3zE1708 = false;  }
        z2zE1804 = z3zE1708;
      }
      unit z3zE1730;
      if (z2zE1804) {
        struct zexception z2zE1805;
        CREATE(zexception)(&z2zE1805);
        zInvalidBlock(&z2zE1805, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1805);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:617.8-617.44");
        KILL(zexception)(&z2zE1805);
        goto end_block_exception_2075;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1805);
      } else {  z3zE1730 = UNIT;  }
      bool z2zE1809;
      {
        bool z2zE1808;
        {
          sail_u256 z2zE1806;
          z2zE1806 = zk_header.zbase_fee;
          sail_u256 z2zE1807;
          z2zE1807 = ztx.zmax_fee;
          z2zE1808 = zword_ule(z2zE1806, z2zE1807);
        }
        z2zE1809 = not(z2zE1808);
      }
      unit z3zE1729;
      if (z2zE1809) {
        struct zexception z2zE1810;
        CREATE(zexception)(&z2zE1810);
        zInvalidBlock(&z2zE1810, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1810);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:620.8-620.44");
        KILL(zexception)(&z2zE1810);
        goto end_block_exception_2075;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1810);
      } else {  z3zE1729 = UNIT;  }
      bool z2zE1816;
      {
        bool z2zE1815;
        {
          uint64_t z2zE1812;
          {
            struct zBlobHashes z2zE1811;
            z2zE1811 = ztx.zblob_hashes;
            z2zE1812 = z2zE1811.zcount;
          }
          z2zE1815 = (z2zE1812 != UINT64_C(0));
        }
        bool z3zE1709;
        if (z2zE1815) {
          bool z2zE1814;
          {
            sail_u256 z2zE1813;
            z2zE1813 = ztx.zmax_blob_fee;
            z2zE1814 = zword_ule(zblob_price, z2zE1813);
          }
          z3zE1709 = not(z2zE1814);
        } else {  z3zE1709 = false;  }
        z2zE1816 = z3zE1709;
      }
      unit z3zE1728;
      if (z2zE1816) {
        struct zexception z2zE1817;
        CREATE(zexception)(&z2zE1817);
        zInvalidBlock(&z2zE1817, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1817);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:623.8-623.44");
        KILL(zexception)(&z2zE1817);
        goto end_block_exception_2075;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1817);
      } else {  z3zE1728 = UNIT;  }
      bool z2zE1820;
      {
        bool z2zE1819;
        z2zE1819 = ztx.zis_create;
        bool z3zE1710;
        if (z2zE1819) {
          bool z2zE1818;
          z2zE1818 = zinitcode_sizze_allowedzIreprzGU64zCozKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zinput_len);
          z3zE1710 = not(z2zE1818);
        } else {  z3zE1710 = false;  }
        z2zE1820 = z3zE1710;
      }
      unit z3zE1727;
      if (z2zE1820) {
        struct zexception z2zE1821;
        CREATE(zexception)(&z2zE1821);
        zInvalidBlock(&z2zE1821, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1821);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:626.8-626.44");
        KILL(zexception)(&z2zE1821);
        goto end_block_exception_2075;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1821);
      } else {  z3zE1727 = UNIT;  }
      bool z2zE1825;
      {
        bool z2zE1824;
        {
          sail_u256 z2zE1822;
          z2zE1822 = ztx.zmax_priority_fee;
          sail_u256 z2zE1823;
          z2zE1823 = ztx.zmax_fee;
          z2zE1824 = zword_ule(z2zE1822, z2zE1823);
        }
        z2zE1825 = not(z2zE1824);
      }
      unit z3zE1726;
      if (z2zE1825) {
        struct zexception z2zE1826;
        CREATE(zexception)(&z2zE1826);
        zInvalidBlock(&z2zE1826, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1826);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:629.8-629.44");
        KILL(zexception)(&z2zE1826);
        goto end_block_exception_2075;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1826);
      } else {  z3zE1726 = UNIT;  }
      bool z2zE1829;
      {
        bool z2zE1828;
        {
          enum zTxType z2zE1827;
          z2zE1827 = ztx.ztx_type;
          z2zE1828 = ztx_is_access_list(z2zE1827);
        }
        bool z3zE1711;
        if (z2zE1828) {  z3zE1711 = zfork_lt(zk_fork, zBerlin);  } else {  z3zE1711 = false;  }
        z2zE1829 = z3zE1711;
      }
      unit z3zE1725;
      if (z2zE1829) {
        struct zexception z2zE1830;
        CREATE(zexception)(&z2zE1830);
        zInvalidBlock(&z2zE1830, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1830);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:632.8-632.44");
        KILL(zexception)(&z2zE1830);
        goto end_block_exception_2075;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1830);
      } else {  z3zE1725 = UNIT;  }
      bool z2zE1833;
      {
        bool z2zE1832;
        {
          enum zTxType z2zE1831;
          z2zE1831 = ztx.ztx_type;
          z2zE1832 = ztx_is_dynamic_fee(z2zE1831);
        }
        bool z3zE1712;
        if (z2zE1832) {  z3zE1712 = zfork_lt(zk_fork, zLondon);  } else {  z3zE1712 = false;  }
        z2zE1833 = z3zE1712;
      }
      unit z3zE1724;
      if (z2zE1833) {
        struct zexception z2zE1834;
        CREATE(zexception)(&z2zE1834);
        zInvalidBlock(&z2zE1834, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1834);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:635.8-635.44");
        KILL(zexception)(&z2zE1834);
        goto end_block_exception_2075;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1834);
      } else {  z3zE1724 = UNIT;  }
      bool z2zE1837;
      {
        bool z2zE1836;
        {
          enum zTxType z2zE1835;
          z2zE1835 = ztx.ztx_type;
          z2zE1836 = ztx_is_set_code(z2zE1835);
        }
        bool z3zE1713;
        if (z2zE1836) {  z3zE1713 = ztx.zis_create;  } else {  z3zE1713 = false;  }
        z2zE1837 = z3zE1713;
      }
      unit z3zE1723;
      if (z2zE1837) {
        struct zexception z2zE1838;
        CREATE(zexception)(&z2zE1838);
        zInvalidBlock(&z2zE1838, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1838);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:638.8-638.44");
        KILL(zexception)(&z2zE1838);
        goto end_block_exception_2075;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1838);
      } else {  z3zE1723 = UNIT;  }
      bool z2zE1842;
      {
        bool z2zE1841;
        {
          enum zTxType z2zE1839;
          z2zE1839 = ztx.ztx_type;
          z2zE1841 = ztx_is_set_code(z2zE1839);
        }
        bool z3zE1714;
        if (z2zE1841) {
          uint64_t z2zE1840;
          z2zE1840 = ztx.zauthorizzation_count;
          z3zE1714 = (z2zE1840 == UINT64_C(0));
        } else {  z3zE1714 = false;  }
        z2zE1842 = z3zE1714;
      }
      unit z3zE1722;
      if (z2zE1842) {
        struct zexception z2zE1843;
        CREATE(zexception)(&z2zE1843);
        zInvalidBlock(&z2zE1843, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1843);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:641.8-641.44");
        KILL(zexception)(&z2zE1843);
        goto end_block_exception_2075;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1843);
      } else {  z3zE1722 = UNIT;  }
      bool z2zE1846;
      {
        bool z2zE1845;
        {
          enum zTxType z2zE1844;
          z2zE1844 = ztx.ztx_type;
          z2zE1845 = ztx_is_set_code(z2zE1844);
        }
        bool z3zE1715;
        if (z2zE1845) {  z3zE1715 = zfork_lt(zk_fork, zPrague);  } else {  z3zE1715 = false;  }
        z2zE1846 = z3zE1715;
      }
      unit z3zE1721;
      if (z2zE1846) {
        struct zexception z2zE1847;
        CREATE(zexception)(&z2zE1847);
        zInvalidBlock(&z2zE1847, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1847);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:644.8-644.44");
        KILL(zexception)(&z2zE1847);
        goto end_block_exception_2075;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1847);
      } else {  z3zE1721 = UNIT;  }
      bool z2zE1850;
      {
        enum zTxType z2zE1848;
        z2zE1848 = ztx.ztx_type;
        bool z3zE1716;
        {
          if ((zLegacyTx != z2zE1848)) goto case_2073;
          z3zE1716 = false;
          goto finish_match_2071;
        }
      case_2073: ;
        {
          uint64_t z2zE1849;
          z2zE1849 = ztx.zchain_id;
          z3zE1716 = (z2zE1849 != zk_chain_id);
          goto finish_match_2071;
        }
      case_2072: ;
      finish_match_2071: ;
        z2zE1850 = z3zE1716;
      }
      unit z3zE1720;
      if (z2zE1850) {
        struct zexception z2zE1851;
        CREATE(zexception)(&z2zE1851);
        zInvalidBlock(&z2zE1851, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1851);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:650.8-650.44");
        KILL(zexception)(&z2zE1851);
        goto end_block_exception_2075;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1851);
      } else {  z3zE1720 = UNIT;  }
      bool z2zE1852;
      z2zE1852 = (znonce_before == UINT64_C(18446744073709551615));
      unit z3zE1719;
      if (z2zE1852) {
        struct zexception z2zE1853;
        CREATE(zexception)(&z2zE1853);
        zInvalidBlock(&z2zE1853, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1853);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:653.8-653.44");
        KILL(zexception)(&z2zE1853);
        goto end_block_exception_2075;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1853);
      } else {  z3zE1719 = UNIT;  }
      uint64_t z2zE1854;
      z2zE1854 = zcosts.zintrinsic_execution;
      uint64_t z2zE1855;
      z2zE1855 = zcosts.zintrinsic_state;
      uint64_t z2zE1856;
      z2zE1856 = zcosts.zcalldata_floor;
      sail_u256 z2zE1857;
      z2zE1857 = zcosts.zblob_fee;
      struct zTxValidity z3zE1733;
      z3zE1733.zblob_fee = z2zE1857;
      z3zE1733.zcalldata_floor = z2zE1856;
      z3zE1733.zgas_limit = zgas_limit;
      z3zE1733.zgas_price = zeff_gas_price;
      z3zE1733.zintrinsic_execution_gas = z2zE1854;
      z3zE1733.zintrinsic_state_gas = z2zE1855;
      z3zE1733.znonce_before = znonce_before;
      z3zE1733.zpriority_fee = zeff_priority_fee;
      z3zE1733.zsender = zsender;
      z3zE1696 = z3zE1733;
      goto finish_match_2069;
    }
  case_2070: ;
    sail_match_failure("check_transaction_validity");
  finish_match_2069: ;
    z3zE1691 = z3zE1696;
    goto finish_match_2064;
  }
case_2065: ;
  sail_match_failure("check_transaction_validity");
finish_match_2064: ;
  z8zE468 = z3zE1691;
end_function_2074: ;
  return z8zE468;
end_block_exception_2075: ;
  struct zTxValidity z8zE981 = { .zblob_fee = u256_zero(), .zcalldata_floor = UINT64_C(0xdeadc0de), .zgas_limit = UINT64_C(0xdeadc0de), .zgas_price = u256_zero(), .zintrinsic_execution_gas = UINT64_C(0xdeadc0de), .zintrinsic_state_gas = UINT64_C(0xdeadc0de), .znonce_before = UINT64_C(0xdeadc0de), .zpriority_fee = u256_zero(), .zsender = fixed_bytes_20_zero() };
  return z8zE981;
}

struct zTxUpfrontResult zapply_transaction_upfront_effects(struct zTransaction ztx, struct zTxValidity zv)
{
  struct zTxUpfrontResult z8zE469;
  bool zcreate_target_prestate_empty;
  {
    bool z2zE1746;
    {
      bool z2zE1745;
      z2zE1745 = zfork_gteq(zk_fork, zAmsterdam);
      bool z3zE1683;
      if (z2zE1745) {  z3zE1683 = ztx.zis_create;  } else {  z3zE1683 = false;  }
      z2zE1746 = z3zE1683;
    }
    if (z2zE1746) {
      bool z2zE1750;
      {
        sail_fixed_bytes_20 z2zE1749;
        {
          sail_fixed_bytes_20 z2zE1747;
          z2zE1747 = zv.zsender;
          uint64_t z2zE1748;
          z2zE1748 = zv.znonce_before;
          z2zE1749 = zk_create_addr(z2zE1747, z2zE1748);
        }
        {
          z2zE1750 = zk_account_exists(z2zE1749);
          if (have_exception) {  goto end_block_exception_2063;  }
        }
      }
      zcreate_target_prestate_empty = not(z2zE1750);
    } else {  zcreate_target_prestate_empty = false;  }
  }
  sail_fixed_bytes_20 z2zE1736;
  z2zE1736 = zv.zsender;
  sail_u256 z2zE1737;
  {
    sail_u256 z2zE1734;
    z2zE1734 = zv.zgas_price;
    uint64_t z2zE1735;
    z2zE1735 = zv.zgas_limit;
    {
      z2zE1737 = zvalidated_word_productzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE1734, z2zE1735);
      if (have_exception) {  goto end_block_exception_2063;  }
    }
  }
  unit z3zE1687;
  {
    z3zE1687 = zk_sub_balance(z2zE1736, z2zE1737);
    if (have_exception) {  goto end_block_exception_2063;  }
  }
  bool z2zE1739;
  {
    sail_u256 z2zE1738;
    z2zE1738 = zv.zblob_fee;
    z2zE1739 = zword_nonzzero(z2zE1738);
  }
  unit z3zE1686;
  if (z2zE1739) {
    sail_fixed_bytes_20 z2zE1740;
    z2zE1740 = zv.zsender;
    sail_u256 z2zE1741;
    z2zE1741 = zv.zblob_fee;
    {
      z3zE1686 = zk_sub_balance(z2zE1740, z2zE1741);
      if (have_exception) {  goto end_block_exception_2063;  }
    }
  } else {  z3zE1686 = UNIT;  }
  sail_fixed_bytes_20 z2zE1742;
  z2zE1742 = zv.zsender;
  unit z3zE1685;
  {
    z3zE1685 = zk_bump_nonce(z2zE1742);
    if (have_exception) {  goto end_block_exception_2063;  }
  }
  unit z3zE1684;
  z3zE1684 = zprewarm(ztx);
  __int128 zauthorizzation_refund;
  {
    bool z2zE1743;
    z2zE1743 = zfork_lt(zk_fork, zAmsterdam);
    if (z2zE1743) {
      zz5listz8z5structz0zzAuthorizzzzationz9 z2zE1744;
      CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE1744);
      COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE1744, ztx.zauthorizzations);
      {
        zauthorizzation_refund = zprocess_auth_list(z2zE1744);
        if (have_exception) {
          KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE1744);
          goto end_block_exception_2063;
        }
      }
      KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE1744);
    } else {  zauthorizzation_refund = zGAS_REFUND_ZERO;  }
  }
  struct zTxUpfrontResult z3zE1688;
  z3zE1688.zauthorizzation_refund = zauthorizzation_refund;
  z3zE1688.zcreate_target_prestate_empty = zcreate_target_prestate_empty;
  z8zE469 = z3zE1688;
end_function_2062: ;
  return z8zE469;
end_block_exception_2063: ;
  struct zTxUpfrontResult z8zE982 = { .zauthorizzation_refund = ((__int128)INT64_C(0xdeadc0de)), .zcreate_target_prestate_empty = false };
  return z8zE982;
}

unit zenter_transaction_frame(uint64_t zgas_limit, uint64_t zintrinsic_execution, uint64_t zintrinsic_state)
{
  unit z8zE470;
  zpc = UINT64_C(0);
  unit z3zE1669;
  z3zE1669 = UNIT;
  zcall_depth = UINT64_C(0);
  unit z3zE1668;
  z3zE1668 = UNIT;
  uint64_t zafter_execution_intrinsic;
  {
    zafter_execution_intrinsic = zvalidated_gas_subzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zgas_limit, zintrinsic_execution);
    if (have_exception) {  goto end_block_exception_2061;  }
  }
  uint64_t zavailable_gas;
  {
    zavailable_gas = zvalidated_gas_subzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zafter_execution_intrinsic, zintrinsic_state);
    if (have_exception) {  goto end_block_exception_2061;  }
  }
  bool z2zE1732;
  z2zE1732 = zfork_gteq(zk_fork, zAmsterdam);
  unit z3zE1679;
  if (z2zE1732) {
    uint64_t zexecution_budget;
    {
      zexecution_budget = zvalidated_gas_subzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zAMSTERDAM_TX_MAX_GAS, zintrinsic_execution);
      if (have_exception) {  goto end_block_exception_2061;  }
    }
    bool z2zE1733;
    z2zE1733 = (zavailable_gas < zexecution_budget);
    if (z2zE1733) {
      zgas_remaining = zavailable_gas;
      unit z3zE1682;
      z3zE1682 = UNIT;
      zstate_gas_remaining = zGAS_ZERO;
      z3zE1679 = UNIT;
    } else {
      zgas_remaining = zexecution_budget;
      unit z3zE1681;
      z3zE1681 = UNIT;
      uint64_t zremaining_state_gas;
      {
        zremaining_state_gas = zvalidated_gas_subzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zavailable_gas, zexecution_budget);
        if (have_exception) {  goto end_block_exception_2061;  }
      }
      zstate_gas_remaining = zremaining_state_gas;
      z3zE1679 = UNIT;
    }
  } else {
    zgas_remaining = zavailable_gas;
    unit z3zE1680;
    z3zE1680 = UNIT;
    zstate_gas_remaining = zGAS_ZERO;
    z3zE1679 = UNIT;
  }
  zstate_gas_spilled = zSTATE_GAS_SPILL_ZERO;
  unit z3zE1678;
  z3zE1678 = UNIT;
  struct zMessage z3zE1670;
  z3zE1670.zaddress = zZERO_ADDRESS;
  z3zE1670.zcaller = zZERO_ADDRESS;
  z3zE1670.zcode_address = zZERO_ADDRESS;
  z3zE1670.zdepth = UINT64_C(0);
  z3zE1670.zis_static = false;
  z3zE1670.zstate_gas_reservoir = zstate_gas_remaining;
  z3zE1670.zvalue = zZERO_WORD;
  zmessage = z3zE1670;
  unit z3zE1677;
  z3zE1677 = UNIT;
  unit z3zE1676;
  z3zE1676 = stack_reset(UNIT);
  unit z3zE1675;
  z3zE1675 = zmemory_reset(UNIT);
  unit z3zE1674;
  z3zE1674 = zreturndata_clear(UNIT);
  zcalldata = zEMPTY_SLICE;
  unit z3zE1673;
  z3zE1673 = UNIT;
  zframe_code = zEMPTY_CODE;
  unit z3zE1672;
  z3zE1672 = UNIT;
  zframe_refund = zGAS_REFUND_ZERO;
  unit z3zE1671;
  z3zE1671 = UNIT;
  zRunning(&zframe_status, UNIT);
  z8zE470 = UNIT;
end_function_2060: ;
  return z8zE470;
end_block_exception_2061: ;

  return UNIT;
}

bool zprepare_amsterdam_transaction_dispatch(struct zTransaction ztx, struct zTxValidity zv, struct zTxUpfrontResult zupfront)
{
  bool z8zE471;
  sail_fixed_bytes_20 zcurrent_target;
  {
    bool z2zE1729;
    z2zE1729 = ztx.zis_create;
    if (z2zE1729) {
      sail_fixed_bytes_20 z2zE1730;
      z2zE1730 = zv.zsender;
      uint64_t z2zE1731;
      z2zE1731 = zv.znonce_before;
      zcurrent_target = zk_create_addr(z2zE1730, z2zE1731);
    } else {  zcurrent_target = ztx.zrecipient;  }
  }
  sail_fixed_bytes_20 z2zE1711;
  z2zE1711 = zv.zsender;
  sail_u256 z2zE1712;
  z2zE1712 = ztx.zvalue;
  struct zMessage z3zE1654;
  z3zE1654.zaddress = zcurrent_target;
  z3zE1654.zcaller = z2zE1711;
  z3zE1654.zcode_address = zcurrent_target;
  z3zE1654.zdepth = UINT64_C(0);
  z3zE1654.zis_static = false;
  z3zE1654.zstate_gas_reservoir = zstate_gas_remaining;
  z3zE1654.zvalue = z2zE1712;
  zmessage = z3zE1654;
  unit z3zE1655;
  z3zE1655 = UNIT;
  bool z2zE1713;
  z2zE1713 = ztx.zis_create;
  if (z2zE1713) {
    bool z2zE1714;
    z2zE1714 = zupfront.zcreate_target_prestate_empty;
    unit z3zE1667;
    if (z2zE1714) {
      {
        z3zE1667 = zcharge_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_amsterdam_state_new_account);
        if (have_exception) {  goto end_block_exception_2059;  }
      }
    } else {  z3zE1667 = UNIT;  }
    bool z2zE1715;
    z2zE1715 = zis_running(UNIT);
    unit z3zE1666;
    if (z2zE1715) {
      sail_fixed_bytes_32 z2zE1718;
      {
        struct zByteSliceFields z2zE1717;
        {
          struct zByteSliceFields z2zE1716;
          z2zE1716 = ztx.zinput_src;
          z2zE1717 = ztransaction_initcode_slice(z2zE1716);
        }
        z2zE1718 = zcode_db_insert(z2zE1717, zk_fork);
      }
      {
        zframe_code = zcode_db_resolve(z2zE1718);
        if (have_exception) {  goto end_block_exception_2059;  }
      }
      z3zE1666 = UNIT;
    } else {  z3zE1666 = UNIT;  }
    z8zE471 = false;
  } else {
    zcalldata = ztx.zinput_src;
    unit z3zE1658;
    z3zE1658 = UNIT;
    bool z2zE1722;
    {
      bool z2zE1721;
      {
        sail_u256 z2zE1719;
        z2zE1719 = ztx.zvalue;
        z2zE1721 = zword_nonzzero(z2zE1719);
      }
      bool z3zE1656;
      if (z2zE1721) {
        sail_fixed_bytes_20 z2zE1720;
        z2zE1720 = ztx.zrecipient;
        {
          z3zE1656 = zk_account_is_empty(z2zE1720);
          if (have_exception) {  goto end_block_exception_2059;  }
        }
      } else {  z3zE1656 = false;  }
      z2zE1722 = z3zE1656;
    }
    unit z3zE1657;
    if (z2zE1722) {
      {
        z3zE1657 = zcharge_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_amsterdam_state_new_account);
        if (have_exception) {  goto end_block_exception_2059;  }
      }
    } else {  z3zE1657 = UNIT;  }
    bool zdelegated;
    zdelegated = false;
    sail_fixed_bytes_20 zdelegate;
    zdelegate = zZERO_ADDRESS;
    bool z2zE1723;
    z2zE1723 = zis_running(UNIT);
    unit z3zE1661;
    if (z2zE1723) {
      struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1725;
      {
        sail_fixed_bytes_20 z2zE1724;
        z2zE1724 = ztx.zrecipient;
        {
          z2zE1725 = zk_deleg_target(z2zE1724);
          if (have_exception) {  goto end_block_exception_2059;  }
        }
      }
      unit z3zE1662;
      {
        bool zis_delegated;
        zis_delegated = z2zE1725.ztup0;
        sail_fixed_bytes_20 ztarget;
        ztarget = z2zE1725.ztup1;
        zdelegated = zis_delegated;
        unit z3zE1664;
        z3zE1664 = UNIT;
        zdelegate = ztarget;
        unit z3zE1663;
        z3zE1663 = UNIT;
        if (zdelegated) {
          bool zwarm;
          zwarm = zk_access_account(zdelegate);
          uint64_t z2zE1726;
          z2zE1726 = zaccount_cost(zwarm);
          {
            z3zE1662 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE1726);
            if (have_exception) {  goto end_block_exception_2059;  }
          }
        } else {  z3zE1662 = UNIT;  }
        goto finish_match_2056;
      }
    case_2057: ;
      sail_match_failure("prepare_amsterdam_transaction_dispatch");
    finish_match_2056: ;
      z3zE1661 = z3zE1662;
    } else {  z3zE1661 = UNIT;  }
    bool z2zE1727;
    z2zE1727 = zis_running(UNIT);
    unit z3zE1659;
    if (z2zE1727) {
      unit z3zE1660;
      if (zdelegated) {
        zmessage.zcode_address = zdelegate;
        z3zE1660 = UNIT;
      } else {  z3zE1660 = UNIT;  }
      sail_fixed_bytes_20 z2zE1728;
      z2zE1728 = ztx.zrecipient;
      {
        zframe_code = zexecutable_code(z2zE1728, zdelegated, zdelegate);
        if (have_exception) {  goto end_block_exception_2059;  }
      }
      z3zE1659 = UNIT;
    } else {  z3zE1659 = UNIT;  }
    z8zE471 = zdelegated;
  }
end_function_2058: ;
  return z8zE471;
end_block_exception_2059: ;

  return false;
}

unit zrun_create_transaction_frame(struct zTransaction ztx, sail_fixed_bytes_20 zsender, uint64_t znonce_before)
{
  unit z8zE472;
  sail_fixed_bytes_20 znew_addr;
  znew_addr = zk_create_addr(zsender, znonce_before);
  bool z2zE1690;
  z2zE1690 = zk_access_account(znew_addr);
  unit z3zE1636;
  {
    bool z2zE1691;
    {
      z2zE1691 = zk_account_occupied(znew_addr);
      if (have_exception) {  goto end_block_exception_2055;  }
    }
    if (z2zE1691) {
      {
        z3zE1636 = zexc_halt(zAddressCollision);
        if (have_exception) {  goto end_block_exception_2055;  }
      }
    } else {
      unit z3zE1644;
      {
        z3zE1644 = zk_mark_created(znew_addr);
        if (have_exception) {  goto end_block_exception_2055;  }
      }
      unit z3zE1643;
      {
        z3zE1643 = zk_clear_storage(znew_addr);
        if (have_exception) {  goto end_block_exception_2055;  }
      }
      unit z3zE1642;
      {
        z3zE1642 = zk_bump_nonce(znew_addr);
        if (have_exception) {  goto end_block_exception_2055;  }
      }
      bool z2zE1693;
      {
        sail_u256 z2zE1692;
        z2zE1692 = ztx.zvalue;
        z2zE1693 = zword_nonzzero(z2zE1692);
      }
      unit z3zE1641;
      if (z2zE1693) {
        sail_u256 z2zE1694;
        z2zE1694 = ztx.zvalue;
        {
          z3zE1641 = zk_transfer(zsender, znew_addr, z2zE1694);
          if (have_exception) {  goto end_block_exception_2055;  }
        }
      } else {  z3zE1641 = UNIT;  }
      bool z2zE1695;
      z2zE1695 = zfork_lt(zk_fork, zAmsterdam);
      unit z3zE1638;
      if (z2zE1695) {
        sail_u256 z2zE1696;
        z2zE1696 = ztx.zvalue;
        struct zMessage z3zE1639;
        z3zE1639.zaddress = znew_addr;
        z3zE1639.zcaller = zsender;
        z3zE1639.zcode_address = znew_addr;
        z3zE1639.zdepth = UINT64_C(0);
        z3zE1639.zis_static = false;
        z3zE1639.zstate_gas_reservoir = zstate_gas_remaining;
        z3zE1639.zvalue = z2zE1696;
        zmessage = z3zE1639;
        unit z3zE1640;
        z3zE1640 = UNIT;
        sail_fixed_bytes_32 z2zE1699;
        {
          struct zByteSliceFields z2zE1698;
          {
            struct zByteSliceFields z2zE1697;
            z2zE1697 = ztx.zinput_src;
            z2zE1698 = ztransaction_initcode_slice(z2zE1697);
          }
          z2zE1699 = zcode_db_insert(z2zE1698, zk_fork);
        }
        {
          zframe_code = zcode_db_resolve(z2zE1699);
          if (have_exception) {  goto end_block_exception_2055;  }
        }
        z3zE1638 = UNIT;
      } else {  z3zE1638 = UNIT;  }
      struct zByteSliceFields zdeployed_code;
      {
        zdeployed_code = zinterpret(UNIT);
        if (have_exception) {  goto end_block_exception_2055;  }
      }
      bool z2zE1700;
      z2zE1700 = zframe_succeeded(UNIT);
      if (z2zE1700) {
        uint64_t zdep_len;
        zdep_len = zdeployed_code.zlen;
        uint64_t zdeployed_length;
        zdeployed_length = zdep_len;
        bool z2zE1705;
        {
          bool z2zE1704;
          z2zE1704 = zdeployed_code_sizze_allowedzIreprzGU64zCozKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zdeployed_length);
          bool z3zE1647;
          if (z2zE1704) {
            bool z2zE1703;
            z2zE1703 = zfork_lt(zk_fork, zLondon);
            bool z3zE1646;
            if (z2zE1703) {  z3zE1646 = true;  } else {
              bool z2zE1702;
              z2zE1702 = (zdeployed_length == UINT64_C(0));
              bool z3zE1645;
              if (z2zE1702) {  z3zE1645 = true;  } else {
                uint64_t z2zE1701;
                z2zE1701 = zslice_byte(zdeployed_code, UINT64_C(0));
                z3zE1645 = (z2zE1701 != UINT64_C(0xEF));
              }
              z3zE1646 = z3zE1645;
            }
            z3zE1647 = z3zE1646;
          } else {  z3zE1647 = false;  }
          z2zE1705 = z3zE1647;
        }
        if (z2zE1705) {
          struct zoptionzIU64zK z2zE1706;
          CREATE(zoptionzIU64zK)(&z2zE1706);
          zcode_deployment_execution_cost(&z2zE1706, zdep_len, zgas_remaining);
          unit z3zE1648;
          {
            if (z2zE1706.kind != Kind_zSomezIU64zK) goto case_2053;
            uint64_t zexecution_deposit;
            zexecution_deposit = z2zE1706.variants.zSomezIU64zK;
            {
              zgas_remaining = zgas_sub_or_oog(zgas_remaining, zexecution_deposit);
              if (have_exception) {
                KILL(zoptionzIU64zK)(&z2zE1706);
                goto end_block_exception_2055;
              }
            }
            unit z3zE1652;
            z3zE1652 = UNIT;
            uint64_t z2zE1707;
            {
              z2zE1707 = zcode_deployment_state_cost(zdep_len);
              if (have_exception) {
                KILL(zoptionzIU64zK)(&z2zE1706);
                goto end_block_exception_2055;
              }
            }
            unit z3zE1651;
            {
              z3zE1651 = zcharge_deployment_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE1707);
              if (have_exception) {
                KILL(zoptionzIU64zK)(&z2zE1706);
                goto end_block_exception_2055;
              }
            }
            bool z2zE1708;
            z2zE1708 = zframe_succeeded(UNIT);
            if (z2zE1708) {
              struct zByteSliceFields z2zE1709;
              z2zE1709 = zvalidated_code_slice(zdeployed_code);
              {
                z3zE1648 = zk_deploy_code(znew_addr, z2zE1709);
                if (have_exception) {
                  KILL(zoptionzIU64zK)(&z2zE1706);
                  goto end_block_exception_2055;
                }
              }
            } else {  z3zE1648 = UNIT;  }
            goto finish_match_2051;
          }
        case_2053: ;
          {
            /* complete */
            bool z2zE1710;
            z2zE1710 = zfork_lt(zk_fork, zHomestead);
            if (z2zE1710) {
              zgas_remaining = zGAS_ZERO;
              unit z3zE1650;
              z3zE1650 = UNIT;
              {
                z3zE1648 = zk_deploy_code(znew_addr, zEMPTY_CODE_SLICE);
                if (have_exception) {
                  KILL(zoptionzIU64zK)(&z2zE1706);
                  goto end_block_exception_2055;
                }
              }
            } else {
              {
                z3zE1648 = zexc_halt(zOutOfGas);
                if (have_exception) {
                  KILL(zoptionzIU64zK)(&z2zE1706);
                  goto end_block_exception_2055;
                }
              }
            }
            goto finish_match_2051;
          }
        case_2052: ;
        finish_match_2051: ;
          z3zE1636 = z3zE1648;
          KILL(zoptionzIU64zK)(&z2zE1706);
        } else {
          {
            z3zE1636 = zexc_halt(zOutOfGas);
            if (have_exception) {  goto end_block_exception_2055;  }
          }
        }
      } else {  z3zE1636 = UNIT;  }
    }
    goto finish_match_2049;
  }
case_2050: ;
  sail_match_failure("run_create_transaction_frame");
finish_match_2049: ;
  z8zE472 = z3zE1636;
end_function_2054: ;
  return z8zE472;
end_block_exception_2055: ;

  return UNIT;
}

unit zrun_call_transaction_frame(struct zTransaction ztx, sail_fixed_bytes_20 zsender, bool zdelegated)
{
  unit z8zE473;
  struct zAccount z2zE1665;
  {
    sail_fixed_bytes_20 z2zE1664;
    z2zE1664 = ztx.zrecipient;
    {
      z2zE1665 = zk_aload(z2zE1664);
      if (have_exception) {  goto end_block_exception_2048;  }
    }
  }
  unit z3zE1615;
  {
    bool z2zE1667;
    {
      sail_u256 z2zE1666;
      z2zE1666 = ztx.zvalue;
      z2zE1667 = zword_nonzzero(z2zE1666);
    }
    unit z3zE1616;
    if (z2zE1667) {
      sail_fixed_bytes_20 z2zE1668;
      z2zE1668 = ztx.zrecipient;
      sail_u256 z2zE1669;
      z2zE1669 = ztx.zvalue;
      {
        z3zE1616 = zk_transfer(zsender, z2zE1668, z2zE1669);
        if (have_exception) {  goto end_block_exception_2048;  }
      }
    } else {  z3zE1616 = UNIT;  }
    uint64_t zselected_precompile;
    {
      sail_fixed_bytes_20 z2zE1689;
      z2zE1689 = ztx.zrecipient;
      zselected_precompile = zprecompile_number(z2zE1689);
    }
    bool z2zE1671;
    {
      bool z2zE1670;
      z2zE1670 = not(zdelegated);
      bool z3zE1617;
      if (z2zE1670) {  z3zE1617 = (zselected_precompile != UINT64_C(0));  } else {  z3zE1617 = false;  }
      z2zE1671 = z3zE1617;
    }
    if (z2zE1671) {
      uint64_t znumber;
      znumber = zselected_precompile;
      struct zoptionzIU64zK z2zE1673;
      CREATE(zoptionzIU64zK)(&z2zE1673);
      {
        struct zByteSliceFields z2zE1672;
        z2zE1672 = ztx.zinput_src;
        zprecompile_gas(&z2zE1673, znumber, z2zE1672, zgas_remaining);
      }
      unit z3zE1632;
      {
        if (z2zE1673.kind != Kind_zSomezIU64zK) goto case_2046;
        uint64_t zused;
        zused = z2zE1673.variants.zSomezIU64zK;
        struct zPrecompileResult zresult;
        {
          struct zByteSliceFields z2zE1678;
          z2zE1678 = ztx.zinput_src;
          zresult = zrun_precompile_slice(znumber, z2zE1678);
        }
        bool z2zE1674;
        z2zE1674 = zresult.zsuccess;
        if (z2zE1674) {
          {
            zgas_remaining = zgas_sub_or_oog(zgas_remaining, zused);
            if (have_exception) {
              KILL(zoptionzIU64zK)(&z2zE1673);
              goto end_block_exception_2048;
            }
          }
          unit z3zE1635;
          z3zE1635 = UNIT;
          bool z2zE1675;
          z2zE1675 = zis_running(UNIT);
          if (z2zE1675) {
            struct zHaltKind z2zE1677;
            CREATE(zHaltKind)(&z2zE1677);
            {
              struct zByteSliceFields z2zE1676;
              z2zE1676 = zresult.zoutput;
              zHaltReturn(&z2zE1677, z2zE1676);
            }
            zHalted(&zframe_status, z2zE1677);
            z3zE1632 = UNIT;
            KILL(zHaltKind)(&z2zE1677);
          } else {  z3zE1632 = UNIT;  }
        } else {
          {
            z3zE1632 = zexc_halt(zOutOfGas);
            if (have_exception) {
              KILL(zoptionzIU64zK)(&z2zE1673);
              goto end_block_exception_2048;
            }
          }
        }
        goto finish_match_2044;
      }
    case_2046: ;
      {
        {
          z3zE1632 = zexc_halt(zOutOfGas);
          if (have_exception) {
            KILL(zoptionzIU64zK)(&z2zE1673);
            goto end_block_exception_2048;
          }
        }
        goto finish_match_2044;
      }
    case_2045: ;
    finish_match_2044: ;
      z3zE1615 = z3zE1632;
      KILL(zoptionzIU64zK)(&z2zE1673);
    } else {
      bool z2zE1679;
      z2zE1679 = zfork_lt(zk_fork, zAmsterdam);
      unit z3zE1619;
      if (z2zE1679) {
        zcalldata = ztx.zinput_src;
        unit z3zE1622;
        z3zE1622 = UNIT;
        sail_fixed_bytes_20 z2zE1680;
        z2zE1680 = ztx.zrecipient;
        sail_fixed_bytes_20 z2zE1681;
        z2zE1681 = ztx.zrecipient;
        sail_u256 z2zE1682;
        z2zE1682 = ztx.zvalue;
        struct zMessage z3zE1620;
        z3zE1620.zaddress = z2zE1680;
        z3zE1620.zcaller = zsender;
        z3zE1620.zcode_address = z2zE1681;
        z3zE1620.zdepth = UINT64_C(0);
        z3zE1620.zis_static = false;
        z3zE1620.zstate_gas_reservoir = zstate_gas_remaining;
        z3zE1620.zvalue = z2zE1682;
        zmessage = z3zE1620;
        unit z3zE1621;
        z3zE1621 = UNIT;
        struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1684;
        {
          sail_fixed_bytes_20 z2zE1683;
          z2zE1683 = ztx.zrecipient;
          {
            z2zE1684 = zk_deleg_target(z2zE1683);
            if (have_exception) {  goto end_block_exception_2048;  }
          }
        }
        unit z3zE1623;
        {
          bool ztx_deleg;
          ztx_deleg = z2zE1684.ztup0;
          sail_fixed_bytes_20 ztx_dtgt;
          ztx_dtgt = z2zE1684.ztup1;
          unit z3zE1624;
          if (ztx_deleg) {
            bool z2zE1685;
            z2zE1685 = zk_access_account(ztx_dtgt);
            unit z3zE1625;
            {
              struct zAccount z2zE1686;
              {
                z2zE1686 = zk_aload(ztx_dtgt);
                if (have_exception) {  goto end_block_exception_2048;  }
              }
              unit z3zE1626;
              {
                z3zE1626 = UNIT;
                goto finish_match_2040;
              }
            case_2041: ;
              sail_match_failure("run_call_transaction_frame");
            finish_match_2040: ;
              z3zE1625 = z3zE1626;
              goto finish_match_2038;
            }
          case_2039: ;
            sail_match_failure("run_call_transaction_frame");
          finish_match_2038: ;
            z3zE1624 = z3zE1625;
          } else {  z3zE1624 = UNIT;  }
          sail_fixed_bytes_20 z2zE1687;
          z2zE1687 = ztx.zrecipient;
          {
            zframe_code = zexecutable_code(z2zE1687, ztx_deleg, ztx_dtgt);
            if (have_exception) {  goto end_block_exception_2048;  }
          }
          z3zE1623 = UNIT;
          goto finish_match_2036;
        }
      case_2037: ;
        sail_match_failure("run_call_transaction_frame");
      finish_match_2036: ;
        z3zE1619 = z3zE1623;
      } else {  z3zE1619 = UNIT;  }
      struct zByteSliceFields z2zE1688;
      {
        z2zE1688 = zinterpret(UNIT);
        if (have_exception) {  goto end_block_exception_2048;  }
      }
      unit z3zE1630;
      {
        z3zE1630 = UNIT;
        goto finish_match_2042;
      }
    case_2043: ;
      sail_match_failure("run_call_transaction_frame");
    finish_match_2042: ;
      z3zE1615 = z3zE1630;
    }
    goto finish_match_2034;
  }
case_2035: ;
  sail_match_failure("run_call_transaction_frame");
finish_match_2034: ;
  z8zE473 = z3zE1615;
end_function_2047: ;
  return z8zE473;
end_block_exception_2048: ;

  return UNIT;
}

struct zTxFrameResult zrun_legacy_transaction_frame(struct zTransaction ztx, struct zTxValidity zv)
{
  struct zTxFrameResult z8zE474;
  uint64_t zcheckpoint;
  zcheckpoint = zk_state_checkpoint(UNIT);
  uint64_t z2zE1654;
  z2zE1654 = zv.zgas_limit;
  uint64_t z2zE1655;
  z2zE1655 = zv.zintrinsic_execution_gas;
  uint64_t z2zE1656;
  z2zE1656 = zv.zintrinsic_state_gas;
  unit z3zE1612;
  {
    z3zE1612 = zenter_transaction_frame(z2zE1654, z2zE1655, z2zE1656);
    if (have_exception) {  goto end_block_exception_2033;  }
  }
  bool z2zE1657;
  z2zE1657 = ztx.zis_create;
  unit z3zE1611;
  if (z2zE1657) {
    sail_fixed_bytes_20 z2zE1658;
    z2zE1658 = zv.zsender;
    uint64_t z2zE1659;
    z2zE1659 = zv.znonce_before;
    {
      z3zE1611 = zrun_create_transaction_frame(ztx, z2zE1658, z2zE1659);
      if (have_exception) {  goto end_block_exception_2033;  }
    }
  } else {
    sail_fixed_bytes_20 z2zE1660;
    z2zE1660 = zv.zsender;
    {
      z3zE1611 = zrun_call_transaction_frame(ztx, z2zE1660, false);
      if (have_exception) {  goto end_block_exception_2033;  }
    }
  }
  bool zsuccess;
  zsuccess = zframe_succeeded(UNIT);
  bool z2zE1661;
  z2zE1661 = not(zsuccess);
  unit z3zE1613;
  if (z2zE1661) {  z3zE1613 = zk_revert(zcheckpoint);  } else {  z3zE1613 = UNIT;  }
  __int128 z2zE1662;
  z2zE1662 = zframe_state_gas_used(UNIT);
  __int128 z2zE1663;
  if (zsuccess) {  z2zE1663 = zframe_refund;  } else {  z2zE1663 = zGAS_REFUND_ZERO;  }
  struct zTxFrameResult z3zE1614;
  z3zE1614.zexecution_gas_remaining = zgas_remaining;
  z3zE1614.zrefund = z2zE1663;
  z3zE1614.zstate_gas_remaining = zstate_gas_remaining;
  z3zE1614.zstate_gas_used = z2zE1662;
  z3zE1614.zsuccess = zsuccess;
  z8zE474 = z3zE1614;
end_function_2032: ;
  return z8zE474;
end_block_exception_2033: ;
  struct zTxFrameResult z8zE983 = { .zexecution_gas_remaining = UINT64_C(0xdeadc0de), .zrefund = ((__int128)INT64_C(0xdeadc0de)), .zstate_gas_remaining = UINT64_C(0xdeadc0de), .zstate_gas_used = ((__int128)INT64_C(0xdeadc0de)), .zsuccess = false };
  return z8zE983;
}

struct zTxFrameResult zrun_amsterdam_transaction_frame(struct zTransaction ztx, struct zTxValidity zv, struct zTxUpfrontResult zupfront)
{
  struct zTxFrameResult z8zE475;
  uint64_t z2zE1632;
  z2zE1632 = zv.zgas_limit;
  uint64_t z2zE1633;
  z2zE1633 = zv.zintrinsic_execution_gas;
  uint64_t z2zE1634;
  z2zE1634 = zv.zintrinsic_state_gas;
  unit z3zE1595;
  {
    z3zE1595 = zenter_transaction_frame(z2zE1632, z2zE1633, z2zE1634);
    if (have_exception) {  goto end_block_exception_2031;  }
  }
  uint64_t zpreparation_checkpoint;
  zpreparation_checkpoint = zk_state_checkpoint(UNIT);
  uint64_t zpreparation_reservoir;
  zpreparation_reservoir = zstate_gas_remaining;
  sail_fixed_bytes_20 zcurrent_target;
  {
    bool z2zE1651;
    z2zE1651 = ztx.zis_create;
    if (z2zE1651) {
      sail_fixed_bytes_20 z2zE1652;
      z2zE1652 = zv.zsender;
      uint64_t z2zE1653;
      z2zE1653 = zv.znonce_before;
      zcurrent_target = zk_create_addr(z2zE1652, z2zE1653);
    } else {  zcurrent_target = ztx.zrecipient;  }
  }
  struct zAmsterdamAuthorizzationState z2zE1639;
  CREATE(zAmsterdamAuthorizzationState)(&z2zE1639);
  {
    zz5listz8z5structz0zzAuthorizzzzationz9 z2zE1636;
    CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE1636);
    COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE1636, ztx.zauthorizzations);
    sail_fixed_bytes_20 z2zE1637;
    z2zE1637 = zv.zsender;
    bool z2zE1638;
    {
      sail_u256 z2zE1635;
      z2zE1635 = ztx.zvalue;
      z2zE1638 = zword_nonzzero(z2zE1635);
    }
    {
      zprocess_amsterdam_auth_list(&z2zE1639, z2zE1636, z2zE1637, zcurrent_target, z2zE1638, zEMPTY_AMSTERDAM_AUTHORIZATION_STATE);
      if (have_exception) {
        KILL(zAmsterdamAuthorizzationState)(&z2zE1639);
        KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE1636);
        goto end_block_exception_2031;
      }
    }
    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE1636);
  }
  struct zTxFrameResult z3zE1596;
  {
    __int128 zauthorizzation_state_gas;
    zauthorizzation_state_gas = zFRAME_STATE_GAS_DELTA_ZERO;
    bool zdelegated;
    zdelegated = false;
    bool z2zE1640;
    z2zE1640 = zis_running(UNIT);
    unit z3zE1603;
    if (z2zE1640) {
      zauthorizzation_state_gas = zframe_state_gas_used(UNIT);
      unit z3zE1606;
      z3zE1606 = UNIT;
      zmessage.zstate_gas_reservoir = zstate_gas_remaining;
      unit z3zE1605;
      z3zE1605 = UNIT;
      zstate_gas_spilled = zSTATE_GAS_SPILL_ZERO;
      unit z3zE1604;
      z3zE1604 = UNIT;
      {
        zdelegated = zprepare_amsterdam_transaction_dispatch(ztx, zv, zupfront);
        if (have_exception) {
          KILL(zAmsterdamAuthorizzationState)(&z2zE1639);
          goto end_block_exception_2031;
        }
      }
      z3zE1603 = UNIT;
    } else {  z3zE1603 = UNIT;  }
    bool z2zE1642;
    {
      bool z2zE1641;
      z2zE1641 = zis_running(UNIT);
      z2zE1642 = not(z2zE1641);
    }
    unit z3zE1597;
    if (z2zE1642) {
      unit z3zE1601;
      z3zE1601 = zk_revert(zpreparation_checkpoint);
      zmessage.zstate_gas_reservoir = zpreparation_reservoir;
      unit z3zE1600;
      z3zE1600 = UNIT;
      zstate_gas_remaining = zpreparation_reservoir;
      unit z3zE1599;
      z3zE1599 = UNIT;
      zstate_gas_spilled = zSTATE_GAS_SPILL_ZERO;
      unit z3zE1598;
      z3zE1598 = UNIT;
      struct zTxFrameResult z3zE1602;
      z3zE1602.zexecution_gas_remaining = zgas_remaining;
      z3zE1602.zrefund = zGAS_REFUND_ZERO;
      z3zE1602.zstate_gas_remaining = zstate_gas_remaining;
      z3zE1602.zstate_gas_used = zSTATE_GAS_DELTA_ZERO;
      z3zE1602.zsuccess = false;
      z8zE475 = z3zE1602;
      goto cleanup_2029;
      /* unreachable after return */
      goto end_cleanup_2030;
    cleanup_2029: ;
      KILL(zAmsterdamAuthorizzationState)(&z2zE1639);
      goto end_function_2028;
    end_cleanup_2030: ;
    } else {  z3zE1597 = UNIT;  }
    uint64_t zexecution_checkpoint;
    zexecution_checkpoint = zk_state_checkpoint(UNIT);
    bool z2zE1643;
    z2zE1643 = ztx.zis_create;
    unit z3zE1607;
    if (z2zE1643) {
      sail_fixed_bytes_20 z2zE1644;
      z2zE1644 = zv.zsender;
      uint64_t z2zE1645;
      z2zE1645 = zv.znonce_before;
      {
        z3zE1607 = zrun_create_transaction_frame(ztx, z2zE1644, z2zE1645);
        if (have_exception) {
          KILL(zAmsterdamAuthorizzationState)(&z2zE1639);
          goto end_block_exception_2031;
        }
      }
    } else {
      sail_fixed_bytes_20 z2zE1646;
      z2zE1646 = zv.zsender;
      {
        z3zE1607 = zrun_call_transaction_frame(ztx, z2zE1646, zdelegated);
        if (have_exception) {
          KILL(zAmsterdamAuthorizzationState)(&z2zE1639);
          goto end_block_exception_2031;
        }
      }
    }
    bool zsuccess;
    zsuccess = zframe_succeeded(UNIT);
    bool z2zE1647;
    z2zE1647 = not(zsuccess);
    unit z3zE1608;
    if (z2zE1647) {  z3zE1608 = zk_revert(zexecution_checkpoint);  } else {  z3zE1608 = UNIT;  }
    __int128 z2zE1649;
    {
      __int128 z2zE1648;
      z2zE1648 = zframe_state_gas_used(UNIT);
      {    z2zE1649 = (zauthorizzation_state_gas + z2zE1648);
      }
    }
    __int128 z2zE1650;
    if (zsuccess) {  z2zE1650 = zframe_refund;  } else {  z2zE1650 = zGAS_REFUND_ZERO;  }
    struct zTxFrameResult z3zE1609;
    z3zE1609.zexecution_gas_remaining = zgas_remaining;
    z3zE1609.zrefund = z2zE1650;
    z3zE1609.zstate_gas_remaining = zstate_gas_remaining;
    z3zE1609.zstate_gas_used = z2zE1649;
    z3zE1609.zsuccess = zsuccess;
    z3zE1596 = z3zE1609;
    goto finish_match_2026;
  }
case_2027: ;
  sail_match_failure("run_amsterdam_transaction_frame");
finish_match_2026: ;
  z8zE475 = z3zE1596;
  KILL(zAmsterdamAuthorizzationState)(&z2zE1639);
end_function_2028: ;
  return z8zE475;
end_block_exception_2031: ;
  struct zTxFrameResult z8zE984 = { .zexecution_gas_remaining = UINT64_C(0xdeadc0de), .zrefund = ((__int128)INT64_C(0xdeadc0de)), .zstate_gas_remaining = UINT64_C(0xdeadc0de), .zstate_gas_used = ((__int128)INT64_C(0xdeadc0de)), .zsuccess = false };
  return z8zE984;
}

struct zTxFrameResult zrun_transaction_frame(struct zTransaction ztx, struct zTxValidity zv, struct zTxUpfrontResult zupfront)
{
  struct zTxFrameResult z8zE476;
  bool z2zE1631;
  z2zE1631 = zfork_gteq(zk_fork, zAmsterdam);
  if (z2zE1631) {
    {
      z8zE476 = zrun_amsterdam_transaction_frame(ztx, zv, zupfront);
      if (have_exception) {  goto end_block_exception_2025;  }
    }
  } else {
    {
      z8zE476 = zrun_legacy_transaction_frame(ztx, zv);
      if (have_exception) {  goto end_block_exception_2025;  }
    }
  }
end_function_2024: ;
  return z8zE476;
end_block_exception_2025: ;
  struct zTxFrameResult z8zE985 = { .zexecution_gas_remaining = UINT64_C(0xdeadc0de), .zrefund = ((__int128)INT64_C(0xdeadc0de)), .zstate_gas_remaining = UINT64_C(0xdeadc0de), .zstate_gas_used = ((__int128)INT64_C(0xdeadc0de)), .zsuccess = false };
  return z8zE985;
}

uint64_t zadmitted_transaction_state_gas(__int128 zvalue)
{
  uint64_t z8zE477;
  bool z2zE1625;
  z2zE1625 = (!(UINT64_C(0) < zvalue));
  if (z2zE1625) {  z8zE477 = zGAS_ZERO;  } else {
    bool z2zE1626;
    z2zE1626 = (!(UINT64_C(18446744073709551615) < zvalue));
    if (z2zE1626) {  z8zE477 = (uint64_t)(zvalue);  } else {
      struct zexception z2zE1627;
      CREATE(zexception)(&z2zE1627);
      zInvalidBlock(&z2zE1627, zExecutionInvalid);
      COPY(zexception)(current_exception, z2zE1627);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/evm/transaction.sail:1049.12-1049.48");
      KILL(zexception)(&z2zE1627);
      goto end_block_exception_2021;
      /* unreachable after throw */
      KILL(zexception)(&z2zE1627);
    }
  }
end_function_2020: ;
  return z8zE477;
end_block_exception_2021: ;

  return UINT64_C(0xdeadc0de);
}

void zsettle_transaction(struct zReceipt *z8zE478, struct zTransaction ztx, struct zTxValidity zv, __int128 zauthorizzation_refund, struct zTxFrameResult zfr)
{
  uint64_t zgas_left0;
  {
    uint64_t z2zE1623;
    z2zE1623 = zfr.zexecution_gas_remaining;
    uint64_t z2zE1624;
    z2zE1624 = zfr.zstate_gas_remaining;
    {
      zgas_left0 = zvalidated_gas_add(z2zE1623, z2zE1624);
      if (have_exception) {  goto end_block_exception_2019;  }
    }
  }
  uint64_t zgas_used0;
  {
    uint64_t z2zE1622;
    z2zE1622 = zv.zgas_limit;
    {
      zgas_used0 = zvalidated_gas_subzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE1622, zgas_left0);
      if (have_exception) {  goto end_block_exception_2019;  }
    }
  }
  uint64_t zrefund_quotient;
  {
    bool z2zE1621;
    z2zE1621 = zfork_gteq(zk_fork, zLondon);
    if (z2zE1621) {  zrefund_quotient = UINT64_C(5);  } else {  zrefund_quotient = UINT64_C(2);  }
  }
  uint64_t zrefund_cap;
  zrefund_cap = (zgas_used0 / zrefund_quotient);
  __int128 ztotal_refund;
  {
    __int128 z2zE1620;
    z2zE1620 = zfr.zrefund;
    {
      ztotal_refund = zvalidated_refund_add(zauthorizzation_refund, z2zE1620);
      if (have_exception) {  goto end_block_exception_2019;  }
    }
  }
  uint64_t zrefund;
  zrefund = zcapped_transaction_refundzIreprzGI128zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(ztotal_refund, zrefund_cap);
  uint64_t zgas_left1;
  {
    zgas_left1 = zvalidated_gas_add(zgas_left0, zrefund);
    if (have_exception) {  goto end_block_exception_2019;  }
  }
  uint64_t zgas_used1;
  {
    uint64_t z2zE1619;
    z2zE1619 = zv.zgas_limit;
    {
      zgas_used1 = zvalidated_gas_subzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE1619, zgas_left1);
      if (have_exception) {  goto end_block_exception_2019;  }
    }
  }
  uint64_t zfloor;
  {
    bool z2zE1616;
    z2zE1616 = zfork_gteq(zk_fork, zPrague);
    if (z2zE1616) {
      uint64_t zfloor_cost;
      zfloor_cost = zv.zcalldata_floor;
      uint64_t ztx_limit;
      ztx_limit = zv.zgas_limit;
      bool z2zE1617;
      z2zE1617 = (!(ztx_limit < zfloor_cost));
      if (z2zE1617) {  zfloor = zfloor_cost;  } else {
        struct zexception z2zE1618;
        CREATE(zexception)(&z2zE1618);
        zInvalidBlock(&z2zE1618, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1618);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:1084.16-1084.52");
        KILL(zexception)(&z2zE1618);
        goto end_block_exception_2019;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1618);
      }
    } else {  zfloor = UINT64_C(0);  }
  }
  uint64_t zgas_used;
  {
    bool z2zE1615;
    z2zE1615 = (zgas_used1 < zfloor);
    if (z2zE1615) {  zgas_used = zfloor;  } else {  zgas_used = zgas_used1;  }
  }
  uint64_t zgas_left;
  {
    uint64_t z2zE1614;
    z2zE1614 = zv.zgas_limit;
    {
      zgas_left = zvalidated_gas_subzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE1614, zgas_used);
      if (have_exception) {  goto end_block_exception_2019;  }
    }
  }
  __int128 zraw_state_gas;
  {
    __int128 z2zE1612;
    z2zE1612 = zfr.zstate_gas_used;
    uint64_t z2zE1613;
    z2zE1613 = zv.zintrinsic_state_gas;
    {
      __int128 z3zE1589;
      z3zE1589 = (__int128)(z2zE1613);
      zraw_state_gas = (z2zE1612 + z3zE1589);
    }
  }
  uint64_t ztx_state_gas;
  {
    ztx_state_gas = zadmitted_transaction_state_gas(zraw_state_gas);
    if (have_exception) {  goto end_block_exception_2019;  }
  }
  uint64_t zexecution_before_floor;
  zexecution_before_floor = zGAS_ZERO;
  bool z2zE1599;
  z2zE1599 = (!(zgas_used0 < ztx_state_gas));
  unit z3zE1590;
  if (z2zE1599) {
    uint64_t zreduced_execution_gas;
    {
      zreduced_execution_gas = zvalidated_gas_subzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zgas_used0, ztx_state_gas);
      if (have_exception) {  goto end_block_exception_2019;  }
    }
    zexecution_before_floor = zreduced_execution_gas;
    z3zE1590 = UNIT;
  } else {  z3zE1590 = UNIT;  }
  uint64_t zexecution_gas;
  {
    bool z2zE1610;
    z2zE1610 = zfork_gteq(zk_fork, zAmsterdam);
    if (z2zE1610) {
      bool z2zE1611;
      z2zE1611 = (zexecution_before_floor < zfloor);
      if (z2zE1611) {  zexecution_gas = zfloor;  } else {  zexecution_gas = zexecution_before_floor;  }
    } else {  zexecution_gas = zgas_used;  }
  }
  uint64_t zstate_gas;
  {
    bool z2zE1609;
    z2zE1609 = zfork_gteq(zk_fork, zAmsterdam);
    if (z2zE1609) {  zstate_gas = ztx_state_gas;  } else {  zstate_gas = zGAS_ZERO;  }
  }
  sail_fixed_bytes_20 z2zE1601;
  z2zE1601 = zv.zsender;
  sail_u256 z2zE1602;
  {
    sail_u256 z2zE1600;
    z2zE1600 = zv.zgas_price;
    {
      z2zE1602 = zvalidated_word_productzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE1600, zgas_left);
      if (have_exception) {  goto end_block_exception_2019;  }
    }
  }
  unit z3zE1593;
  {
    z3zE1593 = zk_add_balance(z2zE1601, z2zE1602);
    if (have_exception) {  goto end_block_exception_2019;  }
  }
  sail_fixed_bytes_20 z2zE1604;
  z2zE1604 = zk_coinbase(UNIT);
  sail_u256 z2zE1605;
  {
    sail_u256 z2zE1603;
    z2zE1603 = zv.zpriority_fee;
    {
      z2zE1605 = zvalidated_word_productzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE1603, zgas_used);
      if (have_exception) {  goto end_block_exception_2019;  }
    }
  }
  unit z3zE1592;
  {
    z3zE1592 = zk_add_balance(z2zE1604, z2zE1605);
    if (have_exception) {  goto end_block_exception_2019;  }
  }
  unit z3zE1591;
  z3zE1591 = zk_tx_merge(UNIT);
  enum zTxType z2zE1606;
  z2zE1606 = ztx.ztx_type;
  bool z2zE1607;
  z2zE1607 = zfr.zsuccess;
  zz5listz8z5structz0zzLogEntryz9 z2zE1608;
  CREATE(zz5listz8z5structz0zzLogEntryz9)(&z2zE1608);
  logs_read_all(&z2zE1608, UNIT);
  struct zReceipt z3zE1594;
  CREATE(zReceipt)(&z3zE1594);
  z3zE1594.zexecution_gas = zexecution_gas;
  z3zE1594.zgas_used = zgas_used;
  COPY(zz5listz8z5structz0zzLogEntryz9)(&((&z3zE1594)->zlogs), z2zE1608);
  z3zE1594.zstate_gas = zstate_gas;
  z3zE1594.zsuccess = z2zE1607;
  z3zE1594.ztx_type = z2zE1606;
  COPY(zReceipt)((*(&z8zE478)), z3zE1594);
  KILL(zReceipt)(&z3zE1594);
  KILL(zz5listz8z5structz0zzLogEntryz9)(&z2zE1608);
end_function_2018: ;
  goto end_function_4043;
end_block_exception_2019: ;
  goto end_function_4043;
end_function_4043: ;
}

void zprocess_transaction(struct zReceipt *z8zE479, struct zTransaction ztx)
{
  unit z3zE1579;
  z3zE1579 = zcycle_scope_start(zSCOPE_TX_RESET);
  unit z3zE1578;
  z3zE1578 = zk_tx_reset(UNIT);
  unit z3zE1577;
  z3zE1577 = zcycle_scope_end(zSCOPE_TX_RESET);
  unit z3zE1576;
  z3zE1576 = zcycle_scope_start(zSCOPE_TX_VALIDATE);
  struct zTxValidity zvalidity;
  {
    zvalidity = zcheck_transaction_validity(ztx);
    if (have_exception) {  goto end_block_exception_2017;  }
  }
  unit z3zE1583;
  z3zE1583 = zcycle_scope_end(zSCOPE_TX_VALIDATE);
  struct zTxEnv z2zE1597;
  {
    sail_fixed_bytes_20 z2zE1594;
    z2zE1594 = ztx.zsender;
    sail_u256 z2zE1595;
    z2zE1595 = zvalidity.zgas_price;
    struct zBlobHashes z2zE1596;
    z2zE1596 = ztx.zblob_hashes;
    struct zTxEnv z3zE1580;
    z3zE1580.zblob_hashes = z2zE1596;
    z3zE1580.zgas_price = z2zE1595;
    z3zE1580.zorigin = z2zE1594;
    z2zE1597 = z3zE1580;
  }
  unit z3zE1582;
  z3zE1582 = zk_set_tx(z2zE1597);
  unit z3zE1581;
  z3zE1581 = zcycle_scope_start(zSCOPE_TX_UPFRONT);
  struct zTxUpfrontResult zupfront;
  {
    zupfront = zapply_transaction_upfront_effects(ztx, zvalidity);
    if (have_exception) {  goto end_block_exception_2017;  }
  }
  unit z3zE1585;
  z3zE1585 = zcycle_scope_end(zSCOPE_TX_UPFRONT);
  unit z3zE1584;
  z3zE1584 = zcycle_scope_start(zSCOPE_TX_FRAME);
  struct zTxFrameResult zframe_result;
  {
    zframe_result = zrun_transaction_frame(ztx, zvalidity, zupfront);
    if (have_exception) {  goto end_block_exception_2017;  }
  }
  unit z3zE1587;
  z3zE1587 = zcycle_scope_end(zSCOPE_TX_FRAME);
  unit z3zE1586;
  z3zE1586 = zcycle_scope_start(zSCOPE_TX_SETTLE);
  struct zReceipt zreceipt;
  CREATE(zReceipt)(&zreceipt);
  {
    __int128 z2zE1598;
    z2zE1598 = zupfront.zauthorizzation_refund;
    {
      zsettle_transaction(&zreceipt, ztx, zvalidity, z2zE1598, zframe_result);
      if (have_exception) {
        KILL(zReceipt)(&zreceipt);
        goto end_block_exception_2017;
      }
    }
  }
  unit z3zE1588;
  z3zE1588 = zcycle_scope_end(zSCOPE_TX_SETTLE);
  COPY(zReceipt)((*(&z8zE479)), zreceipt);
  KILL(zReceipt)(&zreceipt);
end_function_2016: ;
  goto end_function_4042;
end_block_exception_2017: ;
  goto end_function_4042;
end_function_4042: ;
}

