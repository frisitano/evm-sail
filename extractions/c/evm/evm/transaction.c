/* Generated from sail/evm/transaction.sail. Included by evm.c; do not compile separately. */
#ifndef EVMSAIL_C_UNITY_BUILD
#include "../evm_internal.h"
#endif
void create_letbind_198(void) {

  uint64_t z3zE251;
  z3zE251 = UINT64_C(21000);
  zG_transaction = z3zE251;
let_end_672: ;
}
void kill_letbind_198(void) {
}

void create_letbind_199(void) {

  uint64_t z3zE252;
  z3zE252 = UINT64_C(32000);
  zG_txcreate = z3zE252;
let_end_673: ;
}
void kill_letbind_199(void) {
}

void create_letbind_200(void) {

  uint64_t z3zE253;
  z3zE253 = UINT64_C(4);
  zG_txdatazzero = z3zE253;
let_end_674: ;
}
void kill_letbind_200(void) {
}

void create_letbind_201(void) {

  uint64_t z3zE254;
  z3zE254 = UINT64_C(16);
  zG_txdatanonzzero = z3zE254;
let_end_675: ;
}
void kill_letbind_201(void) {
}

void create_letbind_202(void) {

  uint64_t z3zE255;
  z3zE255 = UINT64_C(2400);
  zG_access_list_address = z3zE255;
let_end_676: ;
}
void kill_letbind_202(void) {
}

void create_letbind_203(void) {

  uint64_t z3zE256;
  z3zE256 = UINT64_C(1900);
  zG_access_list_storage_key = z3zE256;
let_end_677: ;
}
void kill_letbind_203(void) {
}

void create_letbind_204(void) {

  uint64_t z3zE257;
  z3zE257 = UINT64_C(12500);
  zPER_AUTH_BASE = z3zE257;
let_end_678: ;
}
void kill_letbind_204(void) {
}

void create_letbind_205(void) {

  uint64_t z3zE258;
  z3zE258 = UINT64_C(25000);
  zPER_EMPTY_ACCOUNT = z3zE258;
let_end_679: ;
}
void kill_letbind_205(void) {
}

void create_letbind_206(void) {

  uint64_t z3zE259;
  z3zE259 = UINT64_C(12000);
  zAMSTERDAM_TX_BASE = z3zE259;
let_end_680: ;
}
void kill_letbind_206(void) {
}

void create_letbind_207(void) {

  uint64_t z3zE260;
  z3zE260 = UINT64_C(11000);
  zAMSTERDAM_CREATE_ACCESS = z3zE260;
let_end_681: ;
}
void kill_letbind_207(void) {
}

void create_letbind_208(void) {

  uint64_t z3zE261;
  z3zE261 = UINT64_C(3000);
  zAMSTERDAM_COLD_ACCOUNT_ACCESS = z3zE261;
let_end_682: ;
}
void kill_letbind_208(void) {
}

void create_letbind_209(void) {

  uint64_t z3zE262;
  z3zE262 = UINT64_C(4244);
  zAMSTERDAM_TX_VALUE_COST = z3zE262;
let_end_683: ;
}
void kill_letbind_209(void) {
}

void create_letbind_210(void) {

  uint64_t z3zE263;
  z3zE263 = UINT64_C(1756);
  zAMSTERDAM_TRANSFER_LOG_COST = z3zE263;
let_end_684: ;
}
void kill_letbind_210(void) {
}

void create_letbind_211(void) {

  uint64_t z3zE264;
  z3zE264 = UINT64_C(3000);
  zAMSTERDAM_ACCESS_LIST_ADDRESS = z3zE264;
let_end_685: ;
}
void kill_letbind_211(void) {
}

void create_letbind_212(void) {

  uint64_t z3zE265;
  z3zE265 = UINT64_C(3000);
  zAMSTERDAM_ACCESS_LIST_SLOT = z3zE265;
let_end_686: ;
}
void kill_letbind_212(void) {
}

void create_letbind_213(void) {

  uint64_t z3zE266;
  z3zE266 = UINT64_C(1280);
  zAMSTERDAM_ACCESS_LIST_ADDRESS_FLOOR = z3zE266;
let_end_687: ;
}
void kill_letbind_213(void) {
}

void create_letbind_214(void) {

  uint64_t z3zE267;
  z3zE267 = UINT64_C(2048);
  zAMSTERDAM_ACCESS_LIST_SLOT_FLOOR = z3zE267;
let_end_688: ;
}
void kill_letbind_214(void) {
}

void create_letbind_215(void) {

  uint64_t z3zE268;
  z3zE268 = UINT64_C(7816);
  zAMSTERDAM_AUTH_BASE = z3zE268;
let_end_689: ;
}
void kill_letbind_215(void) {
}

void create_letbind_216(void) {

  uint64_t z3zE269;
  z3zE269 = UINT64_C(64);
  zAMSTERDAM_CALLDATA_FLOOR_BYTE = z3zE269;
let_end_690: ;
}
void kill_letbind_216(void) {
}

void create_letbind_217(void) {

  uint64_t z3zE270;
  z3zE270 = zTRANSACTION_EXECUTION_GAS_LIMIT;
  zAMSTERDAM_TX_MAX_GAS = z3zE270;
let_end_691: ;
}
void kill_letbind_217(void) {
}

struct zByteSliceFields ztransaction_initcode_slice(struct zByteSliceFields zinput)
{
  struct zByteSliceFields z8zE449;
  z8zE449 = zcode_slice(zinput);
end_function_2165: ;
  return z8zE449;
end_block_exception_2166: ;
  struct zByteSliceFields z8zE985 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE985;
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
let_end_693: ;
}
void kill_letbind_218(void) {    KILL(zAmsterdamAuthorizzationState)(&zEMPTY_AMSTERDAM_AUTHORIZATION_STATE);
}

uint64_t zcalldata_cost(struct zByteSliceFields zinput)
{
  uint64_t z8zE450;
  uint64_t znonzzeroes;
  znonzzeroes = zslice_count_nonzzero(zinput);
  uint64_t zinput_len;
  zinput_len = zinput.zlen;
  bool z2zE2053;
  z2zE2053 = (!(zinput_len < znonzzeroes));
  if (z2zE2053) {
    uint64_t zzzeroes;
    {    zzzeroes = (zinput_len - znonzzeroes);
    }
    uint64_t z2zE2054;
    {    z2zE2054 = (zG_txdatazzero * zzzeroes);
    }
    uint64_t z2zE2055;
    {    z2zE2055 = (zG_txdatanonzzero * znonzzeroes);
    }
    {    z8zE450 = (z2zE2054 + z2zE2055);
    }
  } else {
    struct zexception z2zE2056;
    CREATE(zexception)(&z2zE2056);
    zInvalidBlock(&z2zE2056, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE2056);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/transaction.sail:105.8-105.44");
    KILL(zexception)(&z2zE2056);
    goto end_block_exception_2164;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2056);
  }
end_function_2163: ;
  return z8zE450;
end_block_exception_2164: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zvalidate_blob_hash_version_at(struct zBlobHashes zhashes, uint64_t zremaining, uint64_t zoffset)
{
  uint64_t z8zE451;
  bool z2zE2048;
  z2zE2048 = (zremaining == UINT64_C(0));
  if (z2zE2048) {  z8zE451 = UINT64_C(0);  } else {
    bool z2zE2051;
    {
      uint64_t z2zE2050;
      {
        struct zByteSliceFields z2zE2049;
        z2zE2049 = zhashes.zbytes;
        z2zE2050 = zslice_byte(z2zE2049, zoffset);
      }
      z2zE2051 = (z2zE2050 != UINT64_C(0x01));
    }
    unit z3zE1828;
    if (z2zE2051) {
      struct zexception z2zE2052;
      CREATE(zexception)(&z2zE2052);
      zInvalidBlock(&z2zE2052, zExecutionInvalid);
      COPY(zexception)(current_exception, z2zE2052);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/evm/transaction.sail:135.12-135.48");
      KILL(zexception)(&z2zE2052);
      goto end_block_exception_2162;
      /* unreachable after throw */
      KILL(zexception)(&z2zE2052);
    } else {  z3zE1828 = UNIT;  }
    z8zE451 = ztransaction_blob_count_decrement(zremaining);
  }
end_function_2161: ;
  return z8zE451;
end_block_exception_2162: ;

  return UINT64_C(0xdeadc0de);
}

unit zvalidate_blob_hash_versions(struct zBlobHashes zhashes)
{
  unit z8zE452;
  uint64_t zremaining;
  zremaining = zhashes.zcount;
  {
    zremaining = zvalidate_blob_hash_version_at(zhashes, zremaining, UINT64_C(1));
    if (have_exception) {  goto end_block_exception_2160;  }
  }
  unit z3zE1827;
  z3zE1827 = UNIT;
  {
    zremaining = zvalidate_blob_hash_version_at(zhashes, zremaining, UINT64_C(34));
    if (have_exception) {  goto end_block_exception_2160;  }
  }
  unit z3zE1826;
  z3zE1826 = UNIT;
  {
    zremaining = zvalidate_blob_hash_version_at(zhashes, zremaining, UINT64_C(67));
    if (have_exception) {  goto end_block_exception_2160;  }
  }
  unit z3zE1825;
  z3zE1825 = UNIT;
  {
    zremaining = zvalidate_blob_hash_version_at(zhashes, zremaining, UINT64_C(100));
    if (have_exception) {  goto end_block_exception_2160;  }
  }
  unit z3zE1824;
  z3zE1824 = UNIT;
  {
    zremaining = zvalidate_blob_hash_version_at(zhashes, zremaining, UINT64_C(133));
    if (have_exception) {  goto end_block_exception_2160;  }
  }
  unit z3zE1823;
  z3zE1823 = UNIT;
  {
    zremaining = zvalidate_blob_hash_version_at(zhashes, zremaining, UINT64_C(166));
    if (have_exception) {  goto end_block_exception_2160;  }
  }
  unit z3zE1822;
  z3zE1822 = UNIT;
  {
    zremaining = zvalidate_blob_hash_version_at(zhashes, zremaining, UINT64_C(199));
    if (have_exception) {  goto end_block_exception_2160;  }
  }
  unit z3zE1821;
  z3zE1821 = UNIT;
  {
    zremaining = zvalidate_blob_hash_version_at(zhashes, zremaining, UINT64_C(232));
    if (have_exception) {  goto end_block_exception_2160;  }
  }
  unit z3zE1820;
  z3zE1820 = UNIT;
  {
    zremaining = zvalidate_blob_hash_version_at(zhashes, zremaining, UINT64_C(265));
    if (have_exception) {  goto end_block_exception_2160;  }
  }
  unit z3zE1819;
  z3zE1819 = UNIT;
  bool z2zE2046;
  z2zE2046 = (zremaining != UINT64_C(0));
  if (z2zE2046) {
    struct zexception z2zE2047;
    CREATE(zexception)(&z2zE2047);
    zInvalidBlock(&z2zE2047, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE2047);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/transaction.sail:156.8-156.44");
    KILL(zexception)(&z2zE2047);
    goto end_block_exception_2160;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2047);
  } else {  z8zE452 = UNIT;  }
end_function_2159: ;
  return z8zE452;
end_block_exception_2160: ;

  return UNIT;
}

uint64_t zlegacy_intrinsic_gas(struct zTransaction ztx)
{
  uint64_t z8zE453;
  uint64_t zdata_cost;
  {
    struct zByteSliceFields z2zE2045;
    z2zE2045 = ztx.zinput_src;
    {
      zdata_cost = zcalldata_cost(z2zE2045);
      if (have_exception) {  goto end_block_exception_2158;  }
    }
  }
  struct zByteSliceFields zinput;
  zinput = ztx.zinput_src;
  uint64_t zinput_len;
  zinput_len = zinput.zlen;
  uint64_t zaddress_cost;
  {
    uint64_t z2zE2044;
    z2zE2044 = ztx.zaccess_list_address_count;
    {    zaddress_cost = (zG_access_list_address * z2zE2044);
    }
  }
  uint64_t zslot_cost;
  {
    uint64_t z2zE2043;
    z2zE2043 = ztx.zaccess_list_slot_count;
    {    zslot_cost = (zG_access_list_storage_key * z2zE2043);
    }
  }
  uint64_t zauth_cost;
  {
    uint64_t z2zE2042;
    z2zE2042 = ztx.zauthorizzation_count;
    {    zauth_cost = (zPER_EMPTY_ACCOUNT * z2zE2042);
    }
  }
  uint64_t zcommon;
  {
    uint64_t z2zE2041;
    {
      uint64_t z2zE2040;
      {
        uint64_t z2zE2039;
        {    z2zE2039 = (zdata_cost + zG_transaction);
        }
        {    z2zE2040 = (z2zE2039 + zaddress_cost);
        }
      }
      {    z2zE2041 = (z2zE2040 + zslot_cost);
      }
    }
    {    zcommon = (z2zE2041 + zauth_cost);
    }
  }
  bool z2zE2036;
  z2zE2036 = ztx.zis_create;
  if (z2zE2036) {
    uint64_t z2zE2037;
    {    z2zE2037 = (zcommon + zG_txcreate);
    }
    uint64_t z2zE2038;
    z2zE2038 = ztransaction_initcode_gas(zinput_len);
    {    z8zE453 = (z2zE2037 + z2zE2038);
    }
  } else {  z8zE453 = zcommon;  }
end_function_2157: ;
  return z8zE453;
end_block_exception_2158: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zlegacy_calldata_floor(struct zByteSliceFields zinput)
{
  uint64_t z8zE454;
  uint64_t znonzzeroes;
  znonzzeroes = zslice_count_nonzzero(zinput);
  uint64_t zinput_len;
  zinput_len = zinput.zlen;
  bool z2zE2031;
  z2zE2031 = (!(zinput_len < znonzzeroes));
  if (z2zE2031) {
    uint64_t zzzeroes;
    {    zzzeroes = (zinput_len - znonzzeroes);
    }
    uint64_t z2zE2034;
    {
      uint64_t z2zE2032;
      {    z2zE2032 = (UINT64_C(10) * zzzeroes);
      }
      uint64_t z2zE2033;
      {    z2zE2033 = (UINT64_C(40) * znonzzeroes);
      }
      {    z2zE2034 = (z2zE2032 + z2zE2033);
      }
    }
    {    z8zE454 = (z2zE2034 + zG_transaction);
    }
  } else {
    struct zexception z2zE2035;
    CREATE(zexception)(&z2zE2035);
    zInvalidBlock(&z2zE2035, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE2035);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/transaction.sail:187.8-187.44");
    KILL(zexception)(&z2zE2035);
    goto end_block_exception_2156;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2035);
  }
end_function_2155: ;
  return z8zE454;
end_block_exception_2156: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zamsterdam_recipient_execution_cost(struct zTransaction ztx)
{
  uint64_t z8zE455;
  bool z2zE2022;
  z2zE2022 = ztx.zis_create;
  if (z2zE2022) {
    bool z2zE2024;
    {
      sail_u256 z2zE2023;
      z2zE2023 = ztx.zvalue;
      z2zE2024 = zword_nonzzero(z2zE2023);
    }
    if (z2zE2024) {
      {    z8zE455 = (zAMSTERDAM_CREATE_ACCESS + zAMSTERDAM_TRANSFER_LOG_COST);
      }
    } else {  z8zE455 = zAMSTERDAM_CREATE_ACCESS;  }
  } else {
    bool z2zE2027;
    {
      sail_fixed_bytes_20 z2zE2025;
      z2zE2025 = ztx.zrecipient;
      sail_fixed_bytes_20 z2zE2026;
      z2zE2026 = ztx.zsender;
      z2zE2027 = zneq_anythingzIR__sail_c_repr_fixed_byteszIC20zKzK(z2zE2025, z2zE2026);
    }
    if (z2zE2027) {
      bool z2zE2029;
      {
        sail_u256 z2zE2028;
        z2zE2028 = ztx.zvalue;
        z2zE2029 = zword_nonzzero(z2zE2028);
      }
      if (z2zE2029) {
        uint64_t z2zE2030;
        {    z2zE2030 = (zAMSTERDAM_COLD_ACCOUNT_ACCESS + zAMSTERDAM_TX_VALUE_COST);
        }
        {    z8zE455 = (z2zE2030 + zAMSTERDAM_TRANSFER_LOG_COST);
        }
      } else {  z8zE455 = zAMSTERDAM_COLD_ACCOUNT_ACCESS;  }
    } else {  z8zE455 = UINT64_C(0);  }
  }
end_function_2153: ;
  return z8zE455;
end_block_exception_2154: ;

  return UINT64_C(0xdeadc0de);
}

struct zIntrinsicGasCost zintrinsic_gas(struct zTransaction ztx)
{
  struct zIntrinsicGasCost z8zE456;
  bool z2zE1997;
  z2zE1997 = zfork_lt(zk_fork, zAmsterdam);
  if (z2zE1997) {
    uint64_t z2zE1998;
    {
      z2zE1998 = zlegacy_intrinsic_gas(ztx);
      if (have_exception) {  goto end_block_exception_2152;  }
    }
    uint64_t z2zE2000;
    {
      struct zByteSliceFields z2zE1999;
      z2zE1999 = ztx.zinput_src;
      {
        z2zE2000 = zlegacy_calldata_floor(z2zE1999);
        if (have_exception) {  goto end_block_exception_2152;  }
      }
    }
    struct zIntrinsicGasCost z3zE1818;
    z3zE1818.zcalldata_floor = z2zE2000;
    z3zE1818.zexecution = z2zE1998;
    z3zE1818.zstate = UINT64_C(0);
    z8zE456 = z3zE1818;
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
      uint64_t z2zE2020;
      {
        uint64_t z2zE2018;
        {
          uint64_t z2zE2016;
          {    z2zE2016 = (zAMSTERDAM_ACCESS_LIST_ADDRESS * zaddress_count);
          }
          uint64_t z2zE2017;
          {    z2zE2017 = (zAMSTERDAM_ACCESS_LIST_SLOT * zslot_count);
          }
          {    z2zE2018 = (z2zE2016 + z2zE2017);
          }
        }
        uint64_t z2zE2019;
        {    z2zE2019 = (zAMSTERDAM_ACCESS_LIST_ADDRESS_FLOOR * zaddress_count);
        }
        {    z2zE2020 = (z2zE2018 + z2zE2019);
        }
      }
      uint64_t z2zE2021;
      {    z2zE2021 = (zAMSTERDAM_ACCESS_LIST_SLOT_FLOOR * zslot_count);
      }
      {    zaccess_execution = (z2zE2020 + z2zE2021);
      }
    }
    uint64_t zauthorizzation_execution;
    {
      uint64_t z2zE2015;
      z2zE2015 = ztx.zauthorizzation_count;
      {    zauthorizzation_execution = (zAMSTERDAM_AUTH_BASE * z2zE2015);
      }
    }
    uint64_t zcreate_execution;
    {
      bool z2zE2013;
      z2zE2013 = ztx.zis_create;
      if (z2zE2013) {
        uint64_t z2zE2014;
        z2zE2014 = zinput.zlen;
        zcreate_execution = ztransaction_initcode_gas(z2zE2014);
      } else {  zcreate_execution = UINT64_C(0);  }
    }
    uint64_t zexecution;
    {
      uint64_t z2zE2012;
      {
        uint64_t z2zE2011;
        {
          uint64_t z2zE2010;
          {
            uint64_t z2zE2009;
            {
              uint64_t z2zE2008;
              {
                struct zByteSliceFields z2zE2007;
                z2zE2007 = ztx.zinput_src;
                {
                  z2zE2008 = zcalldata_cost(z2zE2007);
                  if (have_exception) {  goto end_block_exception_2152;  }
                }
              }
              {    z2zE2009 = (z2zE2008 + zAMSTERDAM_TX_BASE);
              }
            }
            {    z2zE2010 = (z2zE2009 + zrecipient);
            }
          }
          {    z2zE2011 = (z2zE2010 + zaccess_execution);
          }
        }
        {    z2zE2012 = (z2zE2011 + zauthorizzation_execution);
        }
      }
      {    zexecution = (z2zE2012 + zcreate_execution);
      }
    }
    uint64_t zinput_length;
    zinput_length = zinput.zlen;
    uint64_t zfloor;
    {
      uint64_t z2zE2005;
      {
        uint64_t z2zE2003;
        {
          uint64_t z2zE2002;
          {
            uint64_t z2zE2001;
            {    z2zE2001 = (zAMSTERDAM_CALLDATA_FLOOR_BYTE * zinput_length);
            }
            {    z2zE2002 = (z2zE2001 + zAMSTERDAM_TX_BASE);
            }
          }
          {    z2zE2003 = (z2zE2002 + zrecipient);
          }
        }
        uint64_t z2zE2004;
        {    z2zE2004 = (zAMSTERDAM_ACCESS_LIST_ADDRESS_FLOOR * zaddress_count);
        }
        {    z2zE2005 = (z2zE2003 + z2zE2004);
        }
      }
      uint64_t z2zE2006;
      {    z2zE2006 = (zAMSTERDAM_ACCESS_LIST_SLOT_FLOOR * zslot_count);
      }
      {    zfloor = (z2zE2005 + z2zE2006);
      }
    }
    struct zIntrinsicGasCost z3zE1817;
    z3zE1817.zcalldata_floor = zfloor;
    z3zE1817.zexecution = zexecution;
    z3zE1817.zstate = UINT64_C(0);
    z8zE456 = z3zE1817;
  }
end_function_2151: ;
  return z8zE456;
end_block_exception_2152: ;
  struct zIntrinsicGasCost z8zE986 = { .zcalldata_floor = UINT64_C(0xdeadc0de), .zexecution = UINT64_C(0xdeadc0de), .zstate = UINT64_C(0xdeadc0de) };
  return z8zE986;
}

uint64_t zmax_blobs_per_transaction(unit z3zE1816)
{
  uint64_t z8zE457;
  bool z2zE1995;
  z2zE1995 = zfork_gteq(zk_fork, zOsaka);
  if (z2zE1995) {  z8zE457 = UINT64_C(6);  } else {
    bool z2zE1996;
    z2zE1996 = zfork_gteq(zk_fork, zPrague);
    if (z2zE1996) {  z8zE457 = UINT64_C(9);  } else {  z8zE457 = UINT64_C(6);  }
  }
end_function_2149: ;
  return z8zE457;
end_block_exception_2150: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t ztransaction_blob_gas_for_count(uint64_t zcount)
{
  uint64_t z8zE458;
  uint64_t zactive_maximum;
  zactive_maximum = zmax_blobs_per_transaction(UNIT);
  bool z2zE1993;
  {
    bool z2zE1992;
    z2zE1992 = (!(UINT64_C(9) < zactive_maximum));
    bool z3zE1815;
    if (z2zE1992) {  z3zE1815 = (!(zactive_maximum < zcount));  } else {  z3zE1815 = false;  }
    z2zE1993 = z3zE1815;
  }
  if (z2zE1993) {
    {    z8zE458 = (UINT64_C(131072) * zcount);
    }
  } else {
    struct zexception z2zE1994;
    CREATE(zexception)(&z2zE1994);
    zInvalidBlock(&z2zE1994, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE1994);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/transaction.sail:275.8-275.44");
    KILL(zexception)(&z2zE1994);
    goto end_block_exception_2148;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1994);
  }
end_function_2147: ;
  return z8zE458;
end_block_exception_2148: ;

  return UINT64_C(0xdeadc0de);
}

struct zTransactionCosts ztransaction_costs(struct zTransaction ztx, uint64_t zgas_limit, sail_u256 zblob_price)
{
  struct zTransactionCosts z8zE459;
  struct zIntrinsicGasCost zintrinsic;
  {
    zintrinsic = zintrinsic_gas(ztx);
    if (have_exception) {  goto end_block_exception_2146;  }
  }
  uint64_t zblob_gas;
  {
    uint64_t z2zE1991;
    {
      struct zBlobHashes z2zE1990;
      z2zE1990 = ztx.zblob_hashes;
      z2zE1991 = z2zE1990.zcount;
    }
    {
      zblob_gas = ztransaction_blob_gas_for_count(z2zE1991);
      if (have_exception) {  goto end_block_exception_2146;  }
    }
  }
  sail_u256 zblob_fee;
  {
    zblob_fee = zblob_word_mulzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zblob_price, zblob_gas);
    if (have_exception) {  goto end_block_exception_2146;  }
  }
  sail_u256 zexecution_cap;
  {
    sail_u256 z2zE1989;
    z2zE1989 = ztx.zmax_fee;
    {
      zexecution_cap = zblob_word_mulzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE1989, zgas_limit);
      if (have_exception) {  goto end_block_exception_2146;  }
    }
  }
  sail_u256 zblob_cap;
  {
    sail_u256 z2zE1988;
    z2zE1988 = ztx.zmax_blob_fee;
    {
      zblob_cap = zblob_word_mulzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE1988, zblob_gas);
      if (have_exception) {  goto end_block_exception_2146;  }
    }
  }
  sail_u256 zupfront;
  {
    sail_u256 z2zE1987;
    {
      sail_u256 z2zE1986;
      z2zE1986 = ztx.zvalue;
      {
        z2zE1987 = zblob_word_add(zexecution_cap, z2zE1986);
        if (have_exception) {  goto end_block_exception_2146;  }
      }
    }
    {
      zupfront = zblob_word_add(z2zE1987, zblob_cap);
      if (have_exception) {  goto end_block_exception_2146;  }
    }
  }
  uint64_t z2zE1983;
  z2zE1983 = zintrinsic.zexecution;
  uint64_t z2zE1984;
  z2zE1984 = zintrinsic.zstate;
  uint64_t z2zE1985;
  z2zE1985 = zintrinsic.zcalldata_floor;
  struct zTransactionCosts z3zE1814;
  z3zE1814.zblob_fee = zblob_fee;
  z3zE1814.zblob_gas = zblob_gas;
  z3zE1814.zcalldata_floor = z2zE1985;
  z3zE1814.zintrinsic_execution = z2zE1983;
  z3zE1814.zintrinsic_state = z2zE1984;
  z3zE1814.zupfront = zupfront;
  z8zE459 = z3zE1814;
end_function_2145: ;
  return z8zE459;
end_block_exception_2146: ;
  struct zTransactionCosts z8zE987 = { .zblob_fee = u256_zero(), .zblob_gas = UINT64_C(0xdeadc0de), .zcalldata_floor = UINT64_C(0xdeadc0de), .zintrinsic_execution = UINT64_C(0xdeadc0de), .zintrinsic_state = UINT64_C(0xdeadc0de), .zupfront = u256_zero() };
  return z8zE987;
}

uint64_t zvalidated_gas_add(uint64_t zleft_gas, uint64_t zright_gas)
{
  uint64_t z8zE460;
  {
    z8zE460 = zconserved_gas_add(zleft_gas, zright_gas);
    if (have_exception) {  goto end_block_exception_2142;  }
  }
end_function_2141: ;
  return z8zE460;
end_block_exception_2142: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zadmitted_transaction_gas_limit(uint64_t zvalue, uint64_t zblock_limit)
{
  uint64_t z8zE461;
  bool z2zE1979;
  z2zE1979 = (zblock_limit < zvalue);
  if (z2zE1979) {
    struct zexception z2zE1980;
    CREATE(zexception)(&z2zE1980);
    zInvalidBlock(&z2zE1980, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE1980);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/transaction.sail:323.8-323.44");
    KILL(zexception)(&z2zE1980);
    goto end_block_exception_2138;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1980);
  } else {  z8zE461 = zvalue;  }
end_function_2137: ;
  return z8zE461;
end_block_exception_2138: ;

  return UINT64_C(0xdeadc0de);
}

__int128 zprocess_auth(struct zAuthorizzation zau)
{
  __int128 z8zE462;
  __int128 zrefund;
  zrefund = zGAS_REFUND_ZERO;
  sail_fixed_bytes_20 zauthority;
  zauthority = zau.zauthority;
  bool z2zE1967;
  {
    bool z2zE1966;
    z2zE1966 = zau.zvalid_sig;
    bool z3zE1802;
    if (z2zE1966) {
      bool z2zE1965;
      {
        sail_u256 z2zE1962;
        z2zE1962 = zau.zchain_id;
        z2zE1965 = zword_is_zzero(z2zE1962);
      }
      bool z3zE1801;
      if (z2zE1965) {  z3zE1801 = true;  } else {
        sail_u256 z2zE1963;
        z2zE1963 = zau.zchain_id;
        uint64_t z2zE1964;
        z2zE1964 = zword_of_chain_identifier(zk_chain_id);
        z3zE1801 = u256_eq_u64(z2zE1963, z2zE1964);
      }
      z3zE1802 = z3zE1801;
    } else {  z3zE1802 = false;  }
    z2zE1967 = z3zE1802;
  }
  unit z3zE1803;
  if (z2zE1967) {
    bool z2zE1968;
    z2zE1968 = zk_access_account(zauthority);
    unit z3zE1804;
    {
      struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1969;
      {
        z2zE1969 = zk_deleg_target(zauthority);
        if (have_exception) {  goto end_block_exception_2136;  }
      }
      unit z3zE1805;
      {
        bool zis_deleg;
        zis_deleg = z2zE1969.ztup0;
        bool z2zE1975;
        {
          bool z2zE1974;
          {
            bool z2zE1971;
            {
              sail_fixed_bytes_32 z2zE1970;
              {
                z2zE1970 = zk_code_key(zauthority);
                if (have_exception) {  goto end_block_exception_2136;  }
              }
              z2zE1971 = eq_fixed_bytes_32(z2zE1970, zKECCAK_EMPTY);
            }
            bool z3zE1806;
            if (z2zE1971) {  z3zE1806 = true;  } else {  z3zE1806 = zis_deleg;  }
            z2zE1974 = z3zE1806;
          }
          bool z3zE1807;
          if (z2zE1974) {
            uint64_t z2zE1972;
            {
              z2zE1972 = zk_get_nonce(zauthority);
              if (have_exception) {  goto end_block_exception_2136;  }
            }
            uint64_t z2zE1973;
            z2zE1973 = zau.znonce;
            z3zE1807 = (z2zE1972 == z2zE1973);
          } else {  z3zE1807 = false;  }
          z2zE1975 = z3zE1807;
        }
        if (z2zE1975) {
          bool zexisted;
          {
            zexisted = zk_account_exists(zauthority);
            if (have_exception) {  goto end_block_exception_2136;  }
          }
          bool z2zE1977;
          {
            sail_fixed_bytes_20 z2zE1976;
            z2zE1976 = zau.zaddress;
            z2zE1977 = eq_fixed_bytes_20(z2zE1976, zZERO_ADDRESS);
          }
          unit z3zE1810;
          if (z2zE1977) {
            {
              z3zE1810 = zk_clear_code(zauthority);
              if (have_exception) {  goto end_block_exception_2136;  }
            }
          } else {
            sail_fixed_bytes_20 z2zE1978;
            z2zE1978 = zau.zaddress;
            {
              z3zE1810 = zk_set_delegation(zauthority, z2zE1978);
              if (have_exception) {  goto end_block_exception_2136;  }
            }
          }
          unit z3zE1809;
          {
            z3zE1809 = zk_bump_nonce(zauthority);
            if (have_exception) {  goto end_block_exception_2136;  }
          }
          if (zexisted) {
            {
              __int128 z3zE1811;
              z3zE1811 = (__int128)(zPER_EMPTY_ACCOUNT);
              __int128 z3zE1812;
              z3zE1812 = (__int128)(zPER_AUTH_BASE);
              zrefund = (z3zE1811 - z3zE1812);
            }
            z3zE1805 = UNIT;
          } else {  z3zE1805 = UNIT;  }
        } else {  z3zE1805 = UNIT;  }
        goto finish_match_2133;
      }
    case_2134: ;
      sail_match_failure("process_auth");
    finish_match_2133: ;
      z3zE1804 = z3zE1805;
      goto finish_match_2131;
    }
  case_2132: ;
    sail_match_failure("process_auth");
  finish_match_2131: ;
    z3zE1803 = z3zE1804;
  } else {  z3zE1803 = UNIT;  }
  z8zE462 = zrefund;
end_function_2135: ;
  return z8zE462;
end_block_exception_2136: ;

  return ((__int128)INT64_C(0xdeadc0de));
}

__int128 zprocess_auth_list(zz5listz8z5structz0zzAuthorizzzzationz9 zxs)
{
  __int128 z8zE463;
  __int128 z3zE1798;
  {
    if (!((zxs == NULL))) goto case_2128;
    z3zE1798 = zGAS_REFUND_ZERO;
    goto finish_match_2126;
  }
case_2128: ;
  {
    /* complete */
    struct zAuthorizzation za;
    za = (*zxs).hd;
    zz5listz8z5structz0zzAuthorizzzzationz9 zr;
    CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&zr);
    COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&zr, (*zxs).tl);
    __int128 z2zE1960;
    {
      z2zE1960 = zprocess_auth(za);
      if (have_exception) {
        KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zr);
        goto end_block_exception_2130;
      }
    }
    __int128 z2zE1961;
    {
      z2zE1961 = zprocess_auth_list(zr);
      if (have_exception) {
        KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zr);
        goto end_block_exception_2130;
      }
    }
    {
      z3zE1798 = zvalidated_refund_add(z2zE1960, z2zE1961);
      if (have_exception) {
        KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zr);
        goto end_block_exception_2130;
      }
    }
    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zr);
    goto finish_match_2126;
  }
case_2127: ;
finish_match_2126: ;
  z8zE463 = z3zE1798;
end_function_2129: ;
  return z8zE463;
end_block_exception_2130: ;

  return ((__int128)INT64_C(0xdeadc0de));
}

bool zauthorizzation_address_seen(sail_fixed_bytes_20 za, zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 zxs)
{
  bool z8zE464;
  bool z3zE1794;
  {
    if (!((zxs == NULL))) goto case_2123;
    z3zE1794 = false;
    goto finish_match_2121;
  }
case_2123: ;
  {
    /* complete */
    sail_fixed_bytes_20 zh;
    zh = (*zxs).hd;
    zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 zt;
    CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zt);
    COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zt, (*zxs).tl);
    bool z2zE1959;
    z2zE1959 = eq_fixed_bytes_20(za, zh);
    bool z3zE1795;
    if (z2zE1959) {  z3zE1795 = true;  } else {  z3zE1795 = zauthorizzation_address_seen(za, zt);  }
    z3zE1794 = z3zE1795;
    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zt);
    goto finish_match_2121;
  }
case_2122: ;
finish_match_2121: ;
  z8zE464 = z3zE1794;
end_function_2124: ;
  return z8zE464;
end_block_exception_2125: ;

  return false;
}

void zprocess_amsterdam_auth(struct zAmsterdamAuthorizzationState *z8zE465, struct zAuthorizzation zau, sail_fixed_bytes_20 zsender, sail_fixed_bytes_20 zcurrent_target, bool ztransfers_value, struct zAmsterdamAuthorizzationState zauth_state)
{
  struct zAmsterdamAuthorizzationState znext;
  CREATE(zAmsterdamAuthorizzationState)(&znext);
  COPY(zAmsterdamAuthorizzationState)(&znext, zauth_state);
  sail_fixed_bytes_20 zauthority;
  zauthority = zau.zauthority;
  bool z2zE1919;
  {
    bool z2zE1918;
    z2zE1918 = zau.zvalid_sig;
    bool z3zE1767;
    if (z2zE1918) {
      bool z2zE1917;
      {
        sail_u256 z2zE1914;
        z2zE1914 = zau.zchain_id;
        z2zE1917 = zword_is_zzero(z2zE1914);
      }
      bool z3zE1766;
      if (z2zE1917) {  z3zE1766 = true;  } else {
        sail_u256 z2zE1915;
        z2zE1915 = zau.zchain_id;
        uint64_t z2zE1916;
        z2zE1916 = zword_of_chain_identifier(zk_chain_id);
        z3zE1766 = u256_eq_u64(z2zE1915, z2zE1916);
      }
      z3zE1767 = z3zE1766;
    } else {  z3zE1767 = false;  }
    z2zE1919 = z3zE1767;
  }
  unit z3zE1768;
  if (z2zE1919) {
    bool z2zE1920;
    z2zE1920 = zk_access_account(zauthority);
    unit z3zE1769;
    {
      struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1921;
      {
        z2zE1921 = zk_deleg_target(zauthority);
        if (have_exception) {
          KILL(zAmsterdamAuthorizzationState)(&znext);
          goto end_block_exception_2120;
        }
      }
      unit z3zE1770;
      {
        bool zcurrently_delegated;
        zcurrently_delegated = z2zE1921.ztup0;
        bool z2zE1927;
        {
          bool z2zE1926;
          {
            bool z2zE1923;
            {
              sail_fixed_bytes_32 z2zE1922;
              {
                z2zE1922 = zk_code_key(zauthority);
                if (have_exception) {
                  KILL(zAmsterdamAuthorizzationState)(&znext);
                  goto end_block_exception_2120;
                }
              }
              z2zE1923 = eq_fixed_bytes_32(z2zE1922, zKECCAK_EMPTY);
            }
            bool z3zE1771;
            if (z2zE1923) {  z3zE1771 = true;  } else {  z3zE1771 = zcurrently_delegated;  }
            z2zE1926 = z3zE1771;
          }
          bool z3zE1772;
          if (z2zE1926) {
            uint64_t z2zE1924;
            {
              z2zE1924 = zk_get_nonce(zauthority);
              if (have_exception) {
                KILL(zAmsterdamAuthorizzationState)(&znext);
                goto end_block_exception_2120;
              }
            }
            uint64_t z2zE1925;
            z2zE1925 = zau.znonce;
            z3zE1772 = (z2zE1924 == z2zE1925);
          } else {  z3zE1772 = false;  }
          z2zE1927 = z3zE1772;
        }
        if (z2zE1927) {
          bool zseen;
          {
            zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1958;
            CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1958);
            COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1958, zauth_state.zseen_valid_authorities);
            zseen = zauthorizzation_address_seen(zauthority, z2zE1958);
            KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1958);
          }
          bool zdelegated_before_tx;
          if (zseen) {
            zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1957;
            CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1957);
            COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1957, zauth_state.zoriginally_delegated);
            zdelegated_before_tx = zauthorizzation_address_seen(zauthority, z2zE1957);
            KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1957);
          } else {  zdelegated_before_tx = zcurrently_delegated;  }
          bool zalready_written;
          {
            bool z3zE1776;
            if (zseen) {  z3zE1776 = true;  } else {
              bool z2zE1956;
              z2zE1956 = eq_fixed_bytes_20(zauthority, zsender);
              bool z3zE1775;
              if (z2zE1956) {  z3zE1775 = true;  } else {
                bool z3zE1774;
                if (ztransfers_value) {  z3zE1774 = eq_fixed_bytes_20(zauthority, zcurrent_target);  } else {
                  z3zE1774 = false;
                }
                z3zE1775 = z3zE1774;
              }
              z3zE1776 = z3zE1775;
            }
            zalready_written = z3zE1776;
          }
          bool z2zE1929;
          {
            bool z2zE1928;
            {
              z2zE1928 = zk_account_exists(zauthority);
              if (have_exception) {
                KILL(zAmsterdamAuthorizzationState)(&znext);
                goto end_block_exception_2120;
              }
            }
            z2zE1929 = not(z2zE1928);
          }
          unit z3zE1783;
          if (z2zE1929) {
            {
              z3zE1783 = zcharge_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_amsterdam_state_new_account);
              if (have_exception) {
                KILL(zAmsterdamAuthorizzationState)(&znext);
                goto end_block_exception_2120;
              }
            }
          } else {  z3zE1783 = UNIT;  }
          bool z2zE1931;
          {
            bool z2zE1930;
            z2zE1930 = zis_running(UNIT);
            bool z3zE1777;
            if (z2zE1930) {  z3zE1777 = not(zalready_written);  } else {  z3zE1777 = false;  }
            z2zE1931 = z3zE1777;
          }
          unit z3zE1782;
          if (z2zE1931) {
            {
              z3zE1782 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_amsterdam_account_write);
              if (have_exception) {
                KILL(zAmsterdamAuthorizzationState)(&znext);
                goto end_block_exception_2120;
              }
            }
          } else {  z3zE1782 = UNIT;  }
          bool z2zE1938;
          {
            bool z2zE1937;
            z2zE1937 = zis_running(UNIT);
            bool z3zE1780;
            if (z2zE1937) {
              bool z2zE1936;
              {
                sail_fixed_bytes_20 z2zE1932;
                z2zE1932 = zau.zaddress;
                z2zE1936 = zneq_anythingzIR__sail_c_repr_fixed_byteszIC20zKzK(z2zE1932, zZERO_ADDRESS);
              }
              bool z3zE1779;
              if (z2zE1936) {
                bool z2zE1935;
                z2zE1935 = not(zdelegated_before_tx);
                bool z3zE1778;
                if (z2zE1935) {
                  bool z2zE1934;
                  {
                    zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1933;
                    CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1933);
                    COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1933, zauth_state.zdelegation_set_for);
                    z2zE1934 = zauthorizzation_address_seen(zauthority, z2zE1933);
                    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1933);
                  }
                  z3zE1778 = not(z2zE1934);
                } else {  z3zE1778 = false;  }
                z3zE1779 = z3zE1778;
              } else {  z3zE1779 = false;  }
              z3zE1780 = z3zE1779;
            } else {  z3zE1780 = false;  }
            z2zE1938 = z3zE1780;
          }
          unit z3zE1781;
          if (z2zE1938) {
            {
              z3zE1781 = zcharge_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_amsterdam_state_auth_base);
              if (have_exception) {
                KILL(zAmsterdamAuthorizzationState)(&znext);
                goto end_block_exception_2120;
              }
            }
          } else {  z3zE1781 = UNIT;  }
          bool z2zE1939;
          z2zE1939 = zis_running(UNIT);
          if (z2zE1939) {
            bool z2zE1941;
            {
              sail_fixed_bytes_20 z2zE1940;
              z2zE1940 = zau.zaddress;
              z2zE1941 = eq_fixed_bytes_20(z2zE1940, zZERO_ADDRESS);
            }
            unit z3zE1790;
            if (z2zE1941) {
              {
                z3zE1790 = zk_clear_code(zauthority);
                if (have_exception) {
                  KILL(zAmsterdamAuthorizzationState)(&znext);
                  goto end_block_exception_2120;
                }
              }
            } else {
              sail_fixed_bytes_20 z2zE1942;
              z2zE1942 = zau.zaddress;
              {
                z3zE1790 = zk_set_delegation(zauthority, z2zE1942);
                if (have_exception) {
                  KILL(zAmsterdamAuthorizzationState)(&znext);
                  goto end_block_exception_2120;
                }
              }
            }
            unit z3zE1789;
            {
              z3zE1789 = zk_bump_nonce(zauthority);
              if (have_exception) {
                KILL(zAmsterdamAuthorizzationState)(&znext);
                goto end_block_exception_2120;
              }
            }
            zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1944;
            CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1944);
            {
              zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1943;
              CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1943);
              COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1943, znext.zseen_valid_authorities);
              zconsz3z5structz0zz__sail_c_repr_fixed_bytesz820z9(&z2zE1944, zauthority, z2zE1943);
              KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1943);
            }
            COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&znext)->zseen_valid_authorities), z2zE1944);
            unit z3zE1788;
            z3zE1788 = UNIT;
            bool z2zE1946;
            {
              bool z2zE1945;
              z2zE1945 = not(zseen);
              bool z3zE1785;
              if (z2zE1945) {  z3zE1785 = zcurrently_delegated;  } else {  z3zE1785 = false;  }
              z2zE1946 = z3zE1785;
            }
            unit z3zE1786;
            if (z2zE1946) {
              zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1948;
              CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1948);
              {
                zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1947;
                CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1947);
                COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1947, znext.zoriginally_delegated);
                zconsz3z5structz0zz__sail_c_repr_fixed_bytesz820z9(&z2zE1948, zauthority, z2zE1947);
              }
              struct zAmsterdamAuthorizzationState z3zE1787;
              CREATE(zAmsterdamAuthorizzationState)(&z3zE1787);
              COPY(zAmsterdamAuthorizzationState)(&z3zE1787, znext);
              COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE1787)->zoriginally_delegated), z2zE1948);
              COPY(zAmsterdamAuthorizzationState)(&znext, z3zE1787);
              z3zE1786 = UNIT;
            } else {  z3zE1786 = UNIT;  }
            bool z2zE1953;
            {
              bool z2zE1952;
              {
                sail_fixed_bytes_20 z2zE1949;
                z2zE1949 = zau.zaddress;
                z2zE1952 = zneq_anythingzIR__sail_c_repr_fixed_byteszIC20zKzK(z2zE1949, zZERO_ADDRESS);
              }
              bool z3zE1791;
              if (z2zE1952) {
                bool z2zE1951;
                {
                  zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1950;
                  CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1950);
                  COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1950, znext.zdelegation_set_for);
                  z2zE1951 = zauthorizzation_address_seen(zauthority, z2zE1950);
                }
                z3zE1791 = not(z2zE1951);
              } else {  z3zE1791 = false;  }
              z2zE1953 = z3zE1791;
            }
            if (z2zE1953) {
              zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1955;
              CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1955);
              {
                zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1954;
                CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1954);
                COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1954, znext.zdelegation_set_for);
                zconsz3z5structz0zz__sail_c_repr_fixed_bytesz820z9(&z2zE1955, zauthority, z2zE1954);
              }
              struct zAmsterdamAuthorizzationState z3zE1792;
              CREATE(zAmsterdamAuthorizzationState)(&z3zE1792);
              COPY(zAmsterdamAuthorizzationState)(&z3zE1792, znext);
              COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&((&z3zE1792)->zdelegation_set_for), z2zE1955);
              COPY(zAmsterdamAuthorizzationState)(&znext, z3zE1792);
              z3zE1770 = UNIT;
            } else {  z3zE1770 = UNIT;  }
          } else {  z3zE1770 = UNIT;  }
        } else {  z3zE1770 = UNIT;  }
        goto finish_match_2117;
      }
    case_2118: ;
      sail_match_failure("process_amsterdam_auth");
    finish_match_2117: ;
      z3zE1769 = z3zE1770;
      goto finish_match_2115;
    }
  case_2116: ;
    sail_match_failure("process_amsterdam_auth");
  finish_match_2115: ;
    z3zE1768 = z3zE1769;
  } else {  z3zE1768 = UNIT;  }
  COPY(zAmsterdamAuthorizzationState)((*(&z8zE465)), znext);
  KILL(zAmsterdamAuthorizzationState)(&znext);
end_function_2119: ;
  goto end_function_4064;
end_block_exception_2120: ;
  goto end_function_4064;
end_function_4064: ;
}

void zprocess_amsterdam_auth_list(struct zAmsterdamAuthorizzationState *z8zE466, zz5listz8z5structz0zzAuthorizzzzationz9 zxs, sail_fixed_bytes_20 zsender, sail_fixed_bytes_20 zcurrent_target, bool ztransfers_value, struct zAmsterdamAuthorizzationState zauth_state)
{
  struct zAmsterdamAuthorizzationState z3zE1763;
  CREATE(zAmsterdamAuthorizzationState)(&z3zE1763);
  {
    if (!((zxs == NULL))) goto case_2112;
    COPY(zAmsterdamAuthorizzationState)(&z3zE1763, zauth_state);
    goto finish_match_2110;
  }
case_2112: ;
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
        KILL(zAmsterdamAuthorizzationState)(&z3zE1763);
        KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zr);
        KILL(zAmsterdamAuthorizzationState)(&znext);
        goto end_block_exception_2114;
      }
    }
    bool z2zE1913;
    z2zE1913 = zis_running(UNIT);
    if (z2zE1913) {
      {
        zprocess_amsterdam_auth_list(&z3zE1763, zr, zsender, zcurrent_target, ztransfers_value, znext);
        if (have_exception) {
          KILL(zAmsterdamAuthorizzationState)(&z3zE1763);
          KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zr);
          KILL(zAmsterdamAuthorizzationState)(&znext);
          goto end_block_exception_2114;
        }
      }
    } else {  COPY(zAmsterdamAuthorizzationState)(&z3zE1763, znext);  }
    KILL(zAmsterdamAuthorizzationState)(&znext);
    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&zr);
    goto finish_match_2110;
  }
case_2111: ;
finish_match_2110: ;
  COPY(zAmsterdamAuthorizzationState)((*(&z8zE466)), z3zE1763);
  KILL(zAmsterdamAuthorizzationState)(&z3zE1763);
end_function_2113: ;
  goto end_function_4063;
end_block_exception_2114: ;
  goto end_function_4063;
end_function_4063: ;
}

unit zwarm_access_list_addresses(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 zxs)
{
  unit z8zE467;
  unit z3zE1758;
  {
    if (!((zxs == NULL))) goto case_2107;
    z3zE1758 = UNIT;
    goto finish_match_2103;
  }
case_2107: ;
  {
    /* complete */
    sail_fixed_bytes_20 za;
    za = (*zxs).hd;
    zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 zr;
    CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zr);
    COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zr, (*zxs).tl);
    bool z2zE1912;
    z2zE1912 = zk_access_account(za);
    unit z3zE1759;
    {
      z3zE1759 = zwarm_access_list_addresses(zr);
      goto finish_match_2105;
    }
  case_2106: ;
    sail_match_failure("warm_access_list_addresses");
  finish_match_2105: ;
    z3zE1758 = z3zE1759;
    KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&zr);
    goto finish_match_2103;
  }
case_2104: ;
finish_match_2103: ;
  z8zE467 = z3zE1758;
end_function_2108: ;
  return z8zE467;
end_block_exception_2109: ;

  return UNIT;
}

unit zwarm_access_list_slots(zz5listz8z5structz0zzStorageKeyz9 zxs)
{
  unit z8zE468;
  unit z3zE1753;
  {
    if (!((zxs == NULL))) goto case_2100;
    z3zE1753 = UNIT;
    goto finish_match_2096;
  }
case_2100: ;
  {
    /* complete */
    struct zStorageKey zk;
    zk = (*zxs).hd;
    zz5listz8z5structz0zzStorageKeyz9 zr;
    CREATE(zz5listz8z5structz0zzStorageKeyz9)(&zr);
    COPY(zz5listz8z5structz0zzStorageKeyz9)(&zr, (*zxs).tl);
    bool z2zE1911;
    {
      sail_fixed_bytes_20 z2zE1909;
      z2zE1909 = zk.zaddr;
      sail_u256 z2zE1910;
      z2zE1910 = zk.zslot;
      z2zE1911 = zk_slot_is_warm(z2zE1909, z2zE1910);
    }
    unit z3zE1754;
    {
      z3zE1754 = zwarm_access_list_slots(zr);
      goto finish_match_2098;
    }
  case_2099: ;
    sail_match_failure("warm_access_list_slots");
  finish_match_2098: ;
    z3zE1753 = z3zE1754;
    KILL(zz5listz8z5structz0zzStorageKeyz9)(&zr);
    goto finish_match_2096;
  }
case_2097: ;
finish_match_2096: ;
  z8zE468 = z3zE1753;
end_function_2101: ;
  return z8zE468;
end_block_exception_2102: ;

  return UNIT;
}

unit zprewarm(struct zTransaction ztx)
{
  unit z8zE469;
  bool z2zE1895;
  {
    sail_fixed_bytes_20 z2zE1894;
    z2zE1894 = ztx.zsender;
    z2zE1895 = zk_access_account(z2zE1894);
  }
  unit z3zE1733;
  {
    bool z2zE1898;
    {
      bool z2zE1896;
      z2zE1896 = ztx.zis_create;
      if (z2zE1896) {  z2zE1898 = false;  } else {
        sail_fixed_bytes_20 z2zE1897;
        z2zE1897 = ztx.zrecipient;
        z2zE1898 = zk_access_account(z2zE1897);
      }
    }
    unit z3zE1734;
    {
      bool z2zE1899;
      z2zE1899 = zfork_gteq(zk_fork, zShanghai);
      unit z3zE1735;
      if (z2zE1899) {
        bool z2zE1901;
        {
          sail_fixed_bytes_20 z2zE1900;
          z2zE1900 = zk_coinbase(UNIT);
          z2zE1901 = zk_access_account(z2zE1900);
        }
        unit z3zE1736;
        {
          z3zE1736 = UNIT;
          goto finish_match_2086;
        }
      case_2087: ;
        sail_match_failure("prewarm");
      finish_match_2086: ;
        z3zE1735 = z3zE1736;
      } else {  z3zE1735 = UNIT;  }
      zz5vecz8z5u64z9 zprecompile_addresses;
      CREATE(zz5vecz8z5u64z9)(&zprecompile_addresses);
      {
        zz5vecz8z5u64z9 z3zE1738;
        CREATE(zz5vecz8z5u64z9)(&z3zE1738);
        internal_vector_init_zz5vecz8z5u64z9(&z3zE1738, INT64_C(17));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1738, z3zE1738, INT64_C(0), UINT64_C(17));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1738, z3zE1738, INT64_C(1), UINT64_C(16));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1738, z3zE1738, INT64_C(2), UINT64_C(15));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1738, z3zE1738, INT64_C(3), UINT64_C(14));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1738, z3zE1738, INT64_C(4), UINT64_C(13));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1738, z3zE1738, INT64_C(5), UINT64_C(12));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1738, z3zE1738, INT64_C(6), UINT64_C(11));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1738, z3zE1738, INT64_C(7), UINT64_C(10));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1738, z3zE1738, INT64_C(8), UINT64_C(9));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1738, z3zE1738, INT64_C(9), UINT64_C(8));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1738, z3zE1738, INT64_C(10), UINT64_C(7));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1738, z3zE1738, INT64_C(11), UINT64_C(6));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1738, z3zE1738, INT64_C(12), UINT64_C(5));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1738, z3zE1738, INT64_C(13), UINT64_C(4));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1738, z3zE1738, INT64_C(14), UINT64_C(3));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1738, z3zE1738, INT64_C(15), UINT64_C(2));
        internal_vector_update_zz5vecz8z5u64z9(&z3zE1738, z3zE1738, INT64_C(16), UINT64_C(1));
        KILL(zz5vecz8z5u64z9)(&zprecompile_addresses);
        internal_vector_init_zz5vecz8z5u64z9(&zprecompile_addresses, z3zE1738.len);
        for (int z8zE988 = 0; z8zE988 < z3zE1738.len; z8zE988++) {
          uint64_t z8zE989 = z3zE1738.data[z8zE988];
          uint64_t z8zE990;
          z8zE990 = z8zE989;
          zprecompile_addresses.data[z8zE988] = z8zE990;
        }
        KILL(zz5vecz8z5u64z9)(&z3zE1738);
      }
      int64_t z3zE1739;
      {    z3zE1739 = (int64_t)(UINT64_C(0));
      }
      int64_t z3zE1740;
      {    z3zE1740 = (int64_t)(UINT64_C(16));
      }
      int64_t z3zE1741;
      {    z3zE1741 = (int64_t)(UINT64_C(1));
      }
      {
        int64_t zi;
        zi = z3zE1739;
        unit z3zE1742;
      for_start_2088: ;
        {
          if ((z3zE1740 < zi)) goto for_end_2089;
          uint64_t zp;
          zp = fast_vector_access_zz5vecz8z5u64z9(zprecompile_addresses, zi);
          bool z2zE1902;
          z2zE1902 = zprecompile_active_at_fork(zp);
          if (z2zE1902) {
            bool z2zE1904;
            {
              sail_fixed_bytes_20 z2zE1903;
              z2zE1903 = zprecompile_id_to_address(zp);
              z2zE1904 = zk_access_account(z2zE1903);
            }
            unit z3zE1743;
            {
              z3zE1743 = UNIT;
              goto finish_match_2090;
            }
          case_2091: ;
            sail_match_failure("prewarm");
          finish_match_2090: ;
            z3zE1742 = z3zE1743;
          } else {  z3zE1742 = UNIT;  }
          zi = (zi + z3zE1741);
          goto for_start_2088;
        }
      for_end_2089: ;
      }
      unit z3zE1750;
      z3zE1750 = UNIT;
      bool z2zE1905;
      z2zE1905 = zprecompile_active_at_fork(UINT64_C(256));
      unit z3zE1746;
      if (z2zE1905) {
        bool z2zE1906;
        {
          zz5vecz8z5bv8z9 z3zE1747;
          CREATE(zz5vecz8z5bv8z9)(&z3zE1747);
          internal_vector_init_zz5vecz8z5bv8z9(&z3zE1747, INT64_C(20));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1747, z3zE1747, INT64_C(0), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1747, z3zE1747, INT64_C(1), UINT64_C(0x01));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1747, z3zE1747, INT64_C(2), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1747, z3zE1747, INT64_C(3), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1747, z3zE1747, INT64_C(4), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1747, z3zE1747, INT64_C(5), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1747, z3zE1747, INT64_C(6), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1747, z3zE1747, INT64_C(7), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1747, z3zE1747, INT64_C(8), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1747, z3zE1747, INT64_C(9), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1747, z3zE1747, INT64_C(10), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1747, z3zE1747, INT64_C(11), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1747, z3zE1747, INT64_C(12), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1747, z3zE1747, INT64_C(13), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1747, z3zE1747, INT64_C(14), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1747, z3zE1747, INT64_C(15), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1747, z3zE1747, INT64_C(16), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1747, z3zE1747, INT64_C(17), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1747, z3zE1747, INT64_C(18), UINT64_C(0x00));
          internal_vector_update_zz5vecz8z5bv8z9(&z3zE1747, z3zE1747, INT64_C(19), UINT64_C(0x00));
          {
            sail_fixed_bytes_20 z3zE3866;
            for (size_t z8zE991 = 0; z8zE991 < 20; ++z8zE991) {
              z3zE3866.bytes[z8zE991] = (uint8_t)(z3zE1747.data[z8zE991] & UINT64_C(0xff));
            }
            z2zE1906 = zk_access_account(z3zE3866);
          }
          KILL(zz5vecz8z5bv8z9)(&z3zE1747);
        }
        unit z3zE1748;
        {
          z3zE1748 = UNIT;
          goto finish_match_2092;
        }
      case_2093: ;
        sail_match_failure("prewarm");
      finish_match_2092: ;
        z3zE1746 = z3zE1748;
      } else {  z3zE1746 = UNIT;  }
      zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1907;
      CREATE(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1907);
      COPY(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1907, ztx.zaccess_list_addresses);
      unit z3zE1745;
      z3zE1745 = zwarm_access_list_addresses(z2zE1907);
      KILL(zz5listz8z5structz0zz__sail_c_repr_fixed_bytesz820z9z9)(&z2zE1907);
      zz5listz8z5structz0zzStorageKeyz9 z2zE1908;
      CREATE(zz5listz8z5structz0zzStorageKeyz9)(&z2zE1908);
      COPY(zz5listz8z5structz0zzStorageKeyz9)(&z2zE1908, ztx.zaccess_list_slots);
      z3zE1734 = zwarm_access_list_slots(z2zE1908);
      KILL(zz5listz8z5structz0zzStorageKeyz9)(&z2zE1908);
      KILL(zz5vecz8z5u64z9)(&zprecompile_addresses);
      goto finish_match_2084;
    }
  case_2085: ;
    sail_match_failure("prewarm");
  finish_match_2084: ;
    z3zE1733 = z3zE1734;
    goto finish_match_2082;
  }
case_2083: ;
  sail_match_failure("prewarm");
finish_match_2082: ;
  z8zE469 = z3zE1733;
end_function_2094: ;
  return z8zE469;
end_block_exception_2095: ;

  return UNIT;
}

struct ztuple_z8z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256z9 zeff_gas_price_for(sail_u256 zbase_fee, sail_u256 zmax_fee, sail_u256 zmax_priority_fee)
{
  struct ztuple_z8z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256z9 z8zE470;
  sail_u256 zprice;
  {
    bool z2zE1892;
    z2zE1892 = zword_ule(zmax_fee, zbase_fee);
    if (z2zE1892) {  zprice = zmax_fee;  } else {
      sail_u256 zavailable_priority;
      zavailable_priority = zword_sub_word(zmax_fee, zbase_fee);
      bool z2zE1893;
      z2zE1893 = zword_ule(zmax_priority_fee, zavailable_priority);
      if (z2zE1893) {  zprice = zword_add_word(zbase_fee, zmax_priority_fee);  } else {  zprice = zmax_fee;  }
    }
  }
  sail_u256 zpriority;
  {
    bool z2zE1891;
    z2zE1891 = zword_ule(zbase_fee, zprice);
    if (z2zE1891) {  zpriority = zword_sub_word(zprice, zbase_fee);  } else {  zpriority = zZERO_WORD;  }
  }
  struct ztuple_z8z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256z9 z3zE1732;
  z3zE1732.ztup0 = zprice;
  z3zE1732.ztup1 = zpriority;
  z8zE470 = z3zE1732;
end_function_2080: ;
  return z8zE470;
end_block_exception_2081: ;
  struct ztuple_z8z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256z9 z8zE992 = { .ztup0 = u256_zero(), .ztup1 = u256_zero() };
  return z8zE992;
}

struct zTxValidity zcheck_transaction_validity(struct zTransaction ztx)
{
  struct zTxValidity z8zE471;
  bool z2zE1777;
  {
    bool z2zE1776;
    {
      enum zTxType z2zE1774;
      z2zE1774 = ztx.ztx_type;
      sail_u256 z2zE1775;
      z2zE1775 = ztx.zsig_v;
      z2zE1776 = ztx_sig_v_valid(zk_chain_id, z2zE1774, z2zE1775);
    }
    z2zE1777 = not(z2zE1776);
  }
  unit z3zE1685;
  if (z2zE1777) {
    struct zexception z2zE1778;
    CREATE(zexception)(&z2zE1778);
    zInvalidBlock(&z2zE1778, zInvalidSignature);
    COPY(zexception)(current_exception, z2zE1778);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/transaction.sail:558.8-558.44");
    KILL(zexception)(&z2zE1778);
    goto end_block_exception_2079;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1778);
  } else {  z3zE1685 = UNIT;  }
  uint64_t zparity;
  {
    enum zTxType z2zE1889;
    z2zE1889 = ztx.ztx_type;
    sail_u256 z2zE1890;
    z2zE1890 = ztx.zsig_v;
    zparity = ztx_y_parity(z2zE1889, z2zE1890);
  }
  bool z2zE1784;
  {
    bool z2zE1783;
    {
      sail_fixed_bytes_20 z2zE1779;
      z2zE1779 = ztx.zsender;
      sail_fixed_bytes_32 z2zE1780;
      z2zE1780 = ztx.zsigning_hash;
      sail_u256 z2zE1781;
      z2zE1781 = ztx.zsig_r;
      sail_u256 z2zE1782;
      z2zE1782 = ztx.zsig_s;
      z2zE1783 = ztx_auth_valid(z2zE1779, z2zE1780, zparity, z2zE1781, z2zE1782);
    }
    z2zE1784 = not(z2zE1783);
  }
  unit z3zE1686;
  if (z2zE1784) {
    struct zexception z2zE1785;
    CREATE(zexception)(&z2zE1785);
    zInvalidBlock(&z2zE1785, zInvalidSignature);
    COPY(zexception)(current_exception, z2zE1785);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/transaction.sail:562.8-562.44");
    KILL(zexception)(&z2zE1785);
    goto end_block_exception_2079;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1785);
  } else {  z3zE1686 = UNIT;  }
  uint64_t zgas_limit;
  {
    uint64_t z2zE1887;
    z2zE1887 = ztx.zgas_limit;
    uint64_t z2zE1888;
    z2zE1888 = zk_header.zgas_limit;
    {
      zgas_limit = zadmitted_transaction_gas_limit(z2zE1887, z2zE1888);
      if (have_exception) {  goto end_block_exception_2079;  }
    }
  }
  struct ztuple_z8z5structz0zz__sail_c_repr_u256zCz0z5structz0zz__sail_c_repr_u256z9 z2zE1789;
  {
    sail_u256 z2zE1786;
    z2zE1786 = zk_header.zbase_fee;
    sail_u256 z2zE1787;
    z2zE1787 = ztx.zmax_fee;
    sail_u256 z2zE1788;
    z2zE1788 = ztx.zmax_priority_fee;
    z2zE1789 = zeff_gas_price_for(z2zE1786, z2zE1787, z2zE1788);
  }
  struct zTxValidity z3zE1687;
  {
    sail_u256 zeff_gas_price;
    zeff_gas_price = z2zE1789.ztup0;
    sail_u256 zeff_priority_fee;
    zeff_priority_fee = z2zE1789.ztup1;
    sail_fixed_bytes_20 zsender;
    zsender = ztx.zsender;
    struct zByteSliceFields zinput;
    zinput = ztx.zinput_src;
    uint64_t zinput_len;
    zinput_len = zinput.zlen;
    uint64_t znonce_before;
    {
      znonce_before = zk_get_nonce(zsender);
      if (have_exception) {  goto end_block_exception_2079;  }
    }
    sail_u256 zblob_price;
    {
      uint64_t z2zE1886;
      z2zE1886 = zk_header.zexcess_blob_gas;
      {
        zblob_price = zblob_base_fee(z2zE1886);
        if (have_exception) {  goto end_block_exception_2079;  }
      }
    }
    struct zTransactionCosts zcosts;
    {
      zcosts = ztransaction_costs(ztx, zgas_limit, zblob_price);
      if (have_exception) {  goto end_block_exception_2079;  }
    }
    uint64_t znonce;
    {
      struct zoptionzIU64zK z2zE1884;
      CREATE(zoptionzIU64zK)(&z2zE1884);
      {
        sail_u256 z2zE1883;
        z2zE1883 = ztx.znonce;
        zword_to_account_nonce(&z2zE1884, z2zE1883);
      }
      uint64_t z3zE1688;
      {
        if (z2zE1884.kind != Kind_zSomezIU64zK) goto case_2072;
        uint64_t z1zE1;
        z1zE1 = z2zE1884.variants.zSomezIU64zK;
        z3zE1688 = z1zE1;
        goto finish_match_2070;
      }
    case_2072: ;
      {
        /* complete */
        struct zexception z2zE1885;
        CREATE(zexception)(&z2zE1885);
        zInvalidBlock(&z2zE1885, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1885);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:576.18-576.54");
        KILL(zoptionzIU64zK)(&z2zE1884);
        KILL(zexception)(&z2zE1885);
        goto end_block_exception_2079;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1885);
        goto finish_match_2070;
      }
    case_2071: ;
    finish_match_2070: ;
      znonce = z3zE1688;
      KILL(zoptionzIU64zK)(&z2zE1884);
    }
    bool z2zE1790;
    z2zE1790 = (znonce != znonce_before);
    unit z3zE1691;
    if (z2zE1790) {
      struct zexception z2zE1791;
      CREATE(zexception)(&z2zE1791);
      zInvalidBlock(&z2zE1791, zExecutionInvalid);
      COPY(zexception)(current_exception, z2zE1791);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/evm/transaction.sail:579.8-579.44");
      KILL(zexception)(&z2zE1791);
      goto end_block_exception_2079;
      /* unreachable after throw */
      KILL(zexception)(&z2zE1791);
    } else {  z3zE1691 = UNIT;  }
    struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1792;
    {
      z2zE1792 = zk_deleg_target(zsender);
      if (have_exception) {  goto end_block_exception_2079;  }
    }
    struct zTxValidity z3zE1692;
    {
      bool zsender_deleg;
      zsender_deleg = z2zE1792.ztup0;
      uint64_t zmax_blobs;
      zmax_blobs = zmax_blobs_per_transaction(UNIT);
      bool z2zE1794;
      {
        enum zTxType z2zE1793;
        z2zE1793 = ztx.ztx_type;
        z2zE1794 = ztx_is_blob(z2zE1793);
      }
      unit z3zE1698;
      if (z2zE1794) {
        bool z2zE1802;
        {
          bool z2zE1801;
          z2zE1801 = zfork_lt(zk_fork, zCancun);
          bool z3zE1701;
          if (z2zE1801) {  z3zE1701 = true;  } else {
            bool z2zE1800;
            {
              uint64_t z2zE1796;
              {
                struct zBlobHashes z2zE1795;
                z2zE1795 = ztx.zblob_hashes;
                z2zE1796 = z2zE1795.zcount;
              }
              z2zE1800 = (z2zE1796 == UINT64_C(0));
            }
            bool z3zE1700;
            if (z2zE1800) {  z3zE1700 = true;  } else {
              bool z2zE1799;
              {
                uint64_t z2zE1798;
                {
                  struct zBlobHashes z2zE1797;
                  z2zE1797 = ztx.zblob_hashes;
                  z2zE1798 = z2zE1797.zcount;
                }
                z2zE1799 = (zmax_blobs < z2zE1798);
              }
              bool z3zE1699;
              if (z2zE1799) {  z3zE1699 = true;  } else {  z3zE1699 = ztx.zis_create;  }
              z3zE1700 = z3zE1699;
            }
            z3zE1701 = z3zE1700;
          }
          z2zE1802 = z3zE1701;
        }
        unit z3zE1702;
        if (z2zE1802) {
          struct zexception z2zE1803;
          CREATE(zexception)(&z2zE1803);
          zInvalidBlock(&z2zE1803, zExecutionInvalid);
          COPY(zexception)(current_exception, z2zE1803);
          have_exception = true;
          COPY(sail_string)(throw_location, "sail/evm/transaction.sail:593.12-593.48");
          KILL(zexception)(&z2zE1803);
          goto end_block_exception_2079;
          /* unreachable after throw */
          KILL(zexception)(&z2zE1803);
        } else {  z3zE1702 = UNIT;  }
        struct zBlobHashes z2zE1804;
        z2zE1804 = ztx.zblob_hashes;
        {
          z3zE1698 = zvalidate_blob_hash_versions(z2zE1804);
          if (have_exception) {  goto end_block_exception_2079;  }
        }
      } else {  z3zE1698 = UNIT;  }
      bool z2zE1807;
      {
        bool z2zE1806;
        z2zE1806 = zfork_gteq(zk_fork, zPrague);
        bool z3zE1693;
        if (z2zE1806) {
          uint64_t z2zE1805;
          z2zE1805 = zcosts.zcalldata_floor;
          z3zE1693 = (zgas_limit < z2zE1805);
        } else {  z3zE1693 = false;  }
        z2zE1807 = z3zE1693;
      }
      unit z3zE1697;
      if (z2zE1807) {
        struct zexception z2zE1808;
        CREATE(zexception)(&z2zE1808);
        zInvalidBlock(&z2zE1808, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1808);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:601.8-601.44");
        KILL(zexception)(&z2zE1808);
        goto end_block_exception_2079;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1808);
      } else {  z3zE1697 = UNIT;  }
      bool z2zE1812;
      {
        bool z2zE1811;
        {
          sail_u256 z2zE1809;
          z2zE1809 = zcosts.zupfront;
          sail_u256 z2zE1810;
          {
            z2zE1810 = zk_get_balance(zsender);
            if (have_exception) {  goto end_block_exception_2079;  }
          }
          z2zE1811 = zword_ule(z2zE1809, z2zE1810);
        }
        z2zE1812 = not(z2zE1811);
      }
      unit z3zE1696;
      if (z2zE1812) {
        struct zexception z2zE1813;
        CREATE(zexception)(&z2zE1813);
        zInvalidBlock(&z2zE1813, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1813);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:604.8-604.44");
        KILL(zexception)(&z2zE1813);
        goto end_block_exception_2079;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1813);
      } else {  z3zE1696 = UNIT;  }
      bool z2zE1817;
      {
        bool z2zE1816;
        {
          bool z2zE1815;
          {
            sail_fixed_bytes_32 z2zE1814;
            {
              z2zE1814 = zk_code_key(zsender);
              if (have_exception) {  goto end_block_exception_2079;  }
            }
            z2zE1815 = eq_fixed_bytes_32(z2zE1814, zKECCAK_EMPTY);
          }
          bool z3zE1694;
          if (z2zE1815) {  z3zE1694 = true;  } else {  z3zE1694 = zsender_deleg;  }
          z2zE1816 = z3zE1694;
        }
        z2zE1817 = not(z2zE1816);
      }
      unit z3zE1695;
      if (z2zE1817) {
        struct zexception z2zE1818;
        CREATE(zexception)(&z2zE1818);
        zInvalidBlock(&z2zE1818, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1818);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:607.8-607.44");
        KILL(zexception)(&z2zE1818);
        goto end_block_exception_2079;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1818);
      } else {  z3zE1695 = UNIT;  }
      sail_u128 zintrinsic_total;
      {
        uint64_t z2zE1881;
        z2zE1881 = zcosts.zintrinsic_execution;
        uint64_t z2zE1882;
        z2zE1882 = zcosts.zintrinsic_state;
        zintrinsic_total = u128_add_u64_u64(z2zE1881, z2zE1882);
      }
      bool z2zE1819;
      z2zE1819 = u64_lt_u128(zgas_limit, zintrinsic_total);
      unit z3zE1728;
      if (z2zE1819) {
        struct zexception z2zE1820;
        CREATE(zexception)(&z2zE1820);
        zInvalidBlock(&z2zE1820, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1820);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:611.8-611.44");
        KILL(zexception)(&z2zE1820);
        goto end_block_exception_2079;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1820);
      } else {  z3zE1728 = UNIT;  }
      bool z2zE1823;
      {
        bool z2zE1822;
        z2zE1822 = zfork_gteq(zk_fork, zAmsterdam);
        bool z3zE1703;
        if (z2zE1822) {
          uint64_t z2zE1821;
          z2zE1821 = zcosts.zintrinsic_execution;
          z3zE1703 = (zAMSTERDAM_TX_MAX_GAS < z2zE1821);
        } else {  z3zE1703 = false;  }
        z2zE1823 = z3zE1703;
      }
      unit z3zE1727;
      if (z2zE1823) {
        struct zexception z2zE1824;
        CREATE(zexception)(&z2zE1824);
        zInvalidBlock(&z2zE1824, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1824);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:614.8-614.44");
        KILL(zexception)(&z2zE1824);
        goto end_block_exception_2079;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1824);
      } else {  z3zE1727 = UNIT;  }
      bool z2zE1827;
      {
        bool z2zE1826;
        z2zE1826 = zfork_gteq(zk_fork, zAmsterdam);
        bool z3zE1704;
        if (z2zE1826) {
          uint64_t z2zE1825;
          z2zE1825 = zcosts.zcalldata_floor;
          z3zE1704 = (zAMSTERDAM_TX_MAX_GAS < z2zE1825);
        } else {  z3zE1704 = false;  }
        z2zE1827 = z3zE1704;
      }
      unit z3zE1726;
      if (z2zE1827) {
        struct zexception z2zE1828;
        CREATE(zexception)(&z2zE1828);
        zInvalidBlock(&z2zE1828, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1828);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:617.8-617.44");
        KILL(zexception)(&z2zE1828);
        goto end_block_exception_2079;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1828);
      } else {  z3zE1726 = UNIT;  }
      bool z2zE1832;
      {
        bool z2zE1831;
        {
          sail_u256 z2zE1829;
          z2zE1829 = zk_header.zbase_fee;
          sail_u256 z2zE1830;
          z2zE1830 = ztx.zmax_fee;
          z2zE1831 = zword_ule(z2zE1829, z2zE1830);
        }
        z2zE1832 = not(z2zE1831);
      }
      unit z3zE1725;
      if (z2zE1832) {
        struct zexception z2zE1833;
        CREATE(zexception)(&z2zE1833);
        zInvalidBlock(&z2zE1833, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1833);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:620.8-620.44");
        KILL(zexception)(&z2zE1833);
        goto end_block_exception_2079;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1833);
      } else {  z3zE1725 = UNIT;  }
      bool z2zE1839;
      {
        bool z2zE1838;
        {
          uint64_t z2zE1835;
          {
            struct zBlobHashes z2zE1834;
            z2zE1834 = ztx.zblob_hashes;
            z2zE1835 = z2zE1834.zcount;
          }
          z2zE1838 = (z2zE1835 != UINT64_C(0));
        }
        bool z3zE1705;
        if (z2zE1838) {
          bool z2zE1837;
          {
            sail_u256 z2zE1836;
            z2zE1836 = ztx.zmax_blob_fee;
            z2zE1837 = zword_ule(zblob_price, z2zE1836);
          }
          z3zE1705 = not(z2zE1837);
        } else {  z3zE1705 = false;  }
        z2zE1839 = z3zE1705;
      }
      unit z3zE1724;
      if (z2zE1839) {
        struct zexception z2zE1840;
        CREATE(zexception)(&z2zE1840);
        zInvalidBlock(&z2zE1840, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1840);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:623.8-623.44");
        KILL(zexception)(&z2zE1840);
        goto end_block_exception_2079;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1840);
      } else {  z3zE1724 = UNIT;  }
      bool z2zE1843;
      {
        bool z2zE1842;
        z2zE1842 = ztx.zis_create;
        bool z3zE1706;
        if (z2zE1842) {
          bool z2zE1841;
          z2zE1841 = zinitcode_sizze_allowedzIreprzGU64zCozKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zinput_len);
          z3zE1706 = not(z2zE1841);
        } else {  z3zE1706 = false;  }
        z2zE1843 = z3zE1706;
      }
      unit z3zE1723;
      if (z2zE1843) {
        struct zexception z2zE1844;
        CREATE(zexception)(&z2zE1844);
        zInvalidBlock(&z2zE1844, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1844);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:626.8-626.44");
        KILL(zexception)(&z2zE1844);
        goto end_block_exception_2079;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1844);
      } else {  z3zE1723 = UNIT;  }
      bool z2zE1848;
      {
        bool z2zE1847;
        {
          sail_u256 z2zE1845;
          z2zE1845 = ztx.zmax_priority_fee;
          sail_u256 z2zE1846;
          z2zE1846 = ztx.zmax_fee;
          z2zE1847 = zword_ule(z2zE1845, z2zE1846);
        }
        z2zE1848 = not(z2zE1847);
      }
      unit z3zE1722;
      if (z2zE1848) {
        struct zexception z2zE1849;
        CREATE(zexception)(&z2zE1849);
        zInvalidBlock(&z2zE1849, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1849);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:629.8-629.44");
        KILL(zexception)(&z2zE1849);
        goto end_block_exception_2079;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1849);
      } else {  z3zE1722 = UNIT;  }
      bool z2zE1852;
      {
        bool z2zE1851;
        {
          enum zTxType z2zE1850;
          z2zE1850 = ztx.ztx_type;
          z2zE1851 = ztx_is_access_list(z2zE1850);
        }
        bool z3zE1707;
        if (z2zE1851) {  z3zE1707 = zfork_lt(zk_fork, zBerlin);  } else {  z3zE1707 = false;  }
        z2zE1852 = z3zE1707;
      }
      unit z3zE1721;
      if (z2zE1852) {
        struct zexception z2zE1853;
        CREATE(zexception)(&z2zE1853);
        zInvalidBlock(&z2zE1853, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1853);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:632.8-632.44");
        KILL(zexception)(&z2zE1853);
        goto end_block_exception_2079;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1853);
      } else {  z3zE1721 = UNIT;  }
      bool z2zE1856;
      {
        bool z2zE1855;
        {
          enum zTxType z2zE1854;
          z2zE1854 = ztx.ztx_type;
          z2zE1855 = ztx_is_dynamic_fee(z2zE1854);
        }
        bool z3zE1708;
        if (z2zE1855) {  z3zE1708 = zfork_lt(zk_fork, zLondon);  } else {  z3zE1708 = false;  }
        z2zE1856 = z3zE1708;
      }
      unit z3zE1720;
      if (z2zE1856) {
        struct zexception z2zE1857;
        CREATE(zexception)(&z2zE1857);
        zInvalidBlock(&z2zE1857, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1857);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:635.8-635.44");
        KILL(zexception)(&z2zE1857);
        goto end_block_exception_2079;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1857);
      } else {  z3zE1720 = UNIT;  }
      bool z2zE1860;
      {
        bool z2zE1859;
        {
          enum zTxType z2zE1858;
          z2zE1858 = ztx.ztx_type;
          z2zE1859 = ztx_is_set_code(z2zE1858);
        }
        bool z3zE1709;
        if (z2zE1859) {  z3zE1709 = ztx.zis_create;  } else {  z3zE1709 = false;  }
        z2zE1860 = z3zE1709;
      }
      unit z3zE1719;
      if (z2zE1860) {
        struct zexception z2zE1861;
        CREATE(zexception)(&z2zE1861);
        zInvalidBlock(&z2zE1861, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1861);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:638.8-638.44");
        KILL(zexception)(&z2zE1861);
        goto end_block_exception_2079;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1861);
      } else {  z3zE1719 = UNIT;  }
      bool z2zE1865;
      {
        bool z2zE1864;
        {
          enum zTxType z2zE1862;
          z2zE1862 = ztx.ztx_type;
          z2zE1864 = ztx_is_set_code(z2zE1862);
        }
        bool z3zE1710;
        if (z2zE1864) {
          uint64_t z2zE1863;
          z2zE1863 = ztx.zauthorizzation_count;
          z3zE1710 = (z2zE1863 == UINT64_C(0));
        } else {  z3zE1710 = false;  }
        z2zE1865 = z3zE1710;
      }
      unit z3zE1718;
      if (z2zE1865) {
        struct zexception z2zE1866;
        CREATE(zexception)(&z2zE1866);
        zInvalidBlock(&z2zE1866, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1866);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:641.8-641.44");
        KILL(zexception)(&z2zE1866);
        goto end_block_exception_2079;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1866);
      } else {  z3zE1718 = UNIT;  }
      bool z2zE1869;
      {
        bool z2zE1868;
        {
          enum zTxType z2zE1867;
          z2zE1867 = ztx.ztx_type;
          z2zE1868 = ztx_is_set_code(z2zE1867);
        }
        bool z3zE1711;
        if (z2zE1868) {  z3zE1711 = zfork_lt(zk_fork, zPrague);  } else {  z3zE1711 = false;  }
        z2zE1869 = z3zE1711;
      }
      unit z3zE1717;
      if (z2zE1869) {
        struct zexception z2zE1870;
        CREATE(zexception)(&z2zE1870);
        zInvalidBlock(&z2zE1870, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1870);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:644.8-644.44");
        KILL(zexception)(&z2zE1870);
        goto end_block_exception_2079;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1870);
      } else {  z3zE1717 = UNIT;  }
      bool z2zE1873;
      {
        enum zTxType z2zE1871;
        z2zE1871 = ztx.ztx_type;
        bool z3zE1712;
        {
          if ((zLegacyTx != z2zE1871)) goto case_2077;
          z3zE1712 = false;
          goto finish_match_2075;
        }
      case_2077: ;
        {
          uint64_t z2zE1872;
          z2zE1872 = ztx.zchain_id;
          z3zE1712 = (z2zE1872 != zk_chain_id);
          goto finish_match_2075;
        }
      case_2076: ;
      finish_match_2075: ;
        z2zE1873 = z3zE1712;
      }
      unit z3zE1716;
      if (z2zE1873) {
        struct zexception z2zE1874;
        CREATE(zexception)(&z2zE1874);
        zInvalidBlock(&z2zE1874, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1874);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:650.8-650.44");
        KILL(zexception)(&z2zE1874);
        goto end_block_exception_2079;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1874);
      } else {  z3zE1716 = UNIT;  }
      bool z2zE1875;
      z2zE1875 = (znonce_before == UINT64_C(18446744073709551615));
      unit z3zE1715;
      if (z2zE1875) {
        struct zexception z2zE1876;
        CREATE(zexception)(&z2zE1876);
        zInvalidBlock(&z2zE1876, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1876);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:653.8-653.44");
        KILL(zexception)(&z2zE1876);
        goto end_block_exception_2079;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1876);
      } else {  z3zE1715 = UNIT;  }
      uint64_t z2zE1877;
      z2zE1877 = zcosts.zintrinsic_execution;
      uint64_t z2zE1878;
      z2zE1878 = zcosts.zintrinsic_state;
      uint64_t z2zE1879;
      z2zE1879 = zcosts.zcalldata_floor;
      sail_u256 z2zE1880;
      z2zE1880 = zcosts.zblob_fee;
      struct zTxValidity z3zE1729;
      z3zE1729.zblob_fee = z2zE1880;
      z3zE1729.zcalldata_floor = z2zE1879;
      z3zE1729.zgas_limit = zgas_limit;
      z3zE1729.zgas_price = zeff_gas_price;
      z3zE1729.zintrinsic_execution_gas = z2zE1877;
      z3zE1729.zintrinsic_state_gas = z2zE1878;
      z3zE1729.znonce_before = znonce_before;
      z3zE1729.zpriority_fee = zeff_priority_fee;
      z3zE1729.zsender = zsender;
      z3zE1692 = z3zE1729;
      goto finish_match_2073;
    }
  case_2074: ;
    sail_match_failure("check_transaction_validity");
  finish_match_2073: ;
    z3zE1687 = z3zE1692;
    goto finish_match_2068;
  }
case_2069: ;
  sail_match_failure("check_transaction_validity");
finish_match_2068: ;
  z8zE471 = z3zE1687;
end_function_2078: ;
  return z8zE471;
end_block_exception_2079: ;
  struct zTxValidity z8zE993 = { .zblob_fee = u256_zero(), .zcalldata_floor = UINT64_C(0xdeadc0de), .zgas_limit = UINT64_C(0xdeadc0de), .zgas_price = u256_zero(), .zintrinsic_execution_gas = UINT64_C(0xdeadc0de), .zintrinsic_state_gas = UINT64_C(0xdeadc0de), .znonce_before = UINT64_C(0xdeadc0de), .zpriority_fee = u256_zero(), .zsender = fixed_bytes_20_zero() };
  return z8zE993;
}

struct zTxUpfrontResult zapply_transaction_upfront_effects(struct zTransaction ztx, struct zTxValidity zv)
{
  struct zTxUpfrontResult z8zE472;
  bool zcreate_target_prestate_empty;
  {
    bool z2zE1769;
    {
      bool z2zE1768;
      z2zE1768 = zfork_gteq(zk_fork, zAmsterdam);
      bool z3zE1679;
      if (z2zE1768) {  z3zE1679 = ztx.zis_create;  } else {  z3zE1679 = false;  }
      z2zE1769 = z3zE1679;
    }
    if (z2zE1769) {
      bool z2zE1773;
      {
        sail_fixed_bytes_20 z2zE1772;
        {
          sail_fixed_bytes_20 z2zE1770;
          z2zE1770 = zv.zsender;
          uint64_t z2zE1771;
          z2zE1771 = zv.znonce_before;
          z2zE1772 = zk_create_addr(z2zE1770, z2zE1771);
        }
        {
          z2zE1773 = zk_account_exists(z2zE1772);
          if (have_exception) {  goto end_block_exception_2067;  }
        }
      }
      zcreate_target_prestate_empty = not(z2zE1773);
    } else {  zcreate_target_prestate_empty = false;  }
  }
  sail_fixed_bytes_20 z2zE1759;
  z2zE1759 = zv.zsender;
  sail_u256 z2zE1760;
  {
    sail_u256 z2zE1757;
    z2zE1757 = zv.zgas_price;
    uint64_t z2zE1758;
    z2zE1758 = zv.zgas_limit;
    {
      z2zE1760 = zvalidated_word_productzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE1757, z2zE1758);
      if (have_exception) {  goto end_block_exception_2067;  }
    }
  }
  unit z3zE1683;
  {
    z3zE1683 = zk_sub_balance(z2zE1759, z2zE1760);
    if (have_exception) {  goto end_block_exception_2067;  }
  }
  bool z2zE1762;
  {
    sail_u256 z2zE1761;
    z2zE1761 = zv.zblob_fee;
    z2zE1762 = zword_nonzzero(z2zE1761);
  }
  unit z3zE1682;
  if (z2zE1762) {
    sail_fixed_bytes_20 z2zE1763;
    z2zE1763 = zv.zsender;
    sail_u256 z2zE1764;
    z2zE1764 = zv.zblob_fee;
    {
      z3zE1682 = zk_sub_balance(z2zE1763, z2zE1764);
      if (have_exception) {  goto end_block_exception_2067;  }
    }
  } else {  z3zE1682 = UNIT;  }
  sail_fixed_bytes_20 z2zE1765;
  z2zE1765 = zv.zsender;
  unit z3zE1681;
  {
    z3zE1681 = zk_bump_nonce(z2zE1765);
    if (have_exception) {  goto end_block_exception_2067;  }
  }
  unit z3zE1680;
  z3zE1680 = zprewarm(ztx);
  __int128 zauthorizzation_refund;
  {
    bool z2zE1766;
    z2zE1766 = zfork_lt(zk_fork, zAmsterdam);
    if (z2zE1766) {
      zz5listz8z5structz0zzAuthorizzzzationz9 z2zE1767;
      CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE1767);
      COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE1767, ztx.zauthorizzations);
      {
        zauthorizzation_refund = zprocess_auth_list(z2zE1767);
        if (have_exception) {
          KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE1767);
          goto end_block_exception_2067;
        }
      }
      KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE1767);
    } else {  zauthorizzation_refund = zGAS_REFUND_ZERO;  }
  }
  struct zTxUpfrontResult z3zE1684;
  z3zE1684.zauthorizzation_refund = zauthorizzation_refund;
  z3zE1684.zcreate_target_prestate_empty = zcreate_target_prestate_empty;
  z8zE472 = z3zE1684;
end_function_2066: ;
  return z8zE472;
end_block_exception_2067: ;
  struct zTxUpfrontResult z8zE994 = { .zauthorizzation_refund = ((__int128)INT64_C(0xdeadc0de)), .zcreate_target_prestate_empty = false };
  return z8zE994;
}

unit zenter_transaction_frame(uint64_t zgas_limit, uint64_t zintrinsic_execution, uint64_t zintrinsic_state)
{
  unit z8zE473;
  zpc = UINT64_C(0);
  unit z3zE1665;
  z3zE1665 = UNIT;
  zcall_depth = UINT64_C(0);
  unit z3zE1664;
  z3zE1664 = UNIT;
  uint64_t zafter_execution_intrinsic;
  {
    zafter_execution_intrinsic = zvalidated_gas_subzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zgas_limit, zintrinsic_execution);
    if (have_exception) {  goto end_block_exception_2065;  }
  }
  uint64_t zavailable_gas;
  {
    zavailable_gas = zvalidated_gas_subzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zafter_execution_intrinsic, zintrinsic_state);
    if (have_exception) {  goto end_block_exception_2065;  }
  }
  bool z2zE1755;
  z2zE1755 = zfork_gteq(zk_fork, zAmsterdam);
  unit z3zE1675;
  if (z2zE1755) {
    uint64_t zexecution_budget;
    {
      zexecution_budget = zvalidated_gas_subzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zAMSTERDAM_TX_MAX_GAS, zintrinsic_execution);
      if (have_exception) {  goto end_block_exception_2065;  }
    }
    bool z2zE1756;
    z2zE1756 = (zavailable_gas < zexecution_budget);
    if (z2zE1756) {
      zgas_remaining = zavailable_gas;
      unit z3zE1678;
      z3zE1678 = UNIT;
      zstate_gas_remaining = zGAS_ZERO;
      z3zE1675 = UNIT;
    } else {
      zgas_remaining = zexecution_budget;
      unit z3zE1677;
      z3zE1677 = UNIT;
      uint64_t zremaining_state_gas;
      {
        zremaining_state_gas = zvalidated_gas_subzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zavailable_gas, zexecution_budget);
        if (have_exception) {  goto end_block_exception_2065;  }
      }
      zstate_gas_remaining = zremaining_state_gas;
      z3zE1675 = UNIT;
    }
  } else {
    zgas_remaining = zavailable_gas;
    unit z3zE1676;
    z3zE1676 = UNIT;
    zstate_gas_remaining = zGAS_ZERO;
    z3zE1675 = UNIT;
  }
  zstate_gas_spilled = zSTATE_GAS_SPILL_ZERO;
  unit z3zE1674;
  z3zE1674 = UNIT;
  struct zMessage z3zE1666;
  z3zE1666.zaddress = zZERO_ADDRESS;
  z3zE1666.zcaller = zZERO_ADDRESS;
  z3zE1666.zcode_address = zZERO_ADDRESS;
  z3zE1666.zdepth = UINT64_C(0);
  z3zE1666.zis_static = false;
  z3zE1666.zstate_gas_reservoir = zstate_gas_remaining;
  z3zE1666.zvalue = zZERO_WORD;
  zmessage = z3zE1666;
  unit z3zE1673;
  z3zE1673 = UNIT;
  unit z3zE1672;
  z3zE1672 = stack_reset(UNIT);
  unit z3zE1671;
  z3zE1671 = zmemory_reset(UNIT);
  unit z3zE1670;
  z3zE1670 = zreturndata_clear(UNIT);
  zcalldata = zEMPTY_SLICE;
  unit z3zE1669;
  z3zE1669 = UNIT;
  zframe_code = zEMPTY_CODE;
  unit z3zE1668;
  z3zE1668 = UNIT;
  zframe_refund = zGAS_REFUND_ZERO;
  unit z3zE1667;
  z3zE1667 = UNIT;
  zRunning(&zframe_status, UNIT);
  z8zE473 = UNIT;
end_function_2064: ;
  return z8zE473;
end_block_exception_2065: ;

  return UNIT;
}

bool zprepare_amsterdam_transaction_dispatch(struct zTransaction ztx, struct zTxValidity zv, struct zTxUpfrontResult zupfront)
{
  bool z8zE474;
  sail_fixed_bytes_20 zcurrent_target;
  {
    bool z2zE1752;
    z2zE1752 = ztx.zis_create;
    if (z2zE1752) {
      sail_fixed_bytes_20 z2zE1753;
      z2zE1753 = zv.zsender;
      uint64_t z2zE1754;
      z2zE1754 = zv.znonce_before;
      zcurrent_target = zk_create_addr(z2zE1753, z2zE1754);
    } else {  zcurrent_target = ztx.zrecipient;  }
  }
  sail_fixed_bytes_20 z2zE1734;
  z2zE1734 = zv.zsender;
  sail_u256 z2zE1735;
  z2zE1735 = ztx.zvalue;
  struct zMessage z3zE1650;
  z3zE1650.zaddress = zcurrent_target;
  z3zE1650.zcaller = z2zE1734;
  z3zE1650.zcode_address = zcurrent_target;
  z3zE1650.zdepth = UINT64_C(0);
  z3zE1650.zis_static = false;
  z3zE1650.zstate_gas_reservoir = zstate_gas_remaining;
  z3zE1650.zvalue = z2zE1735;
  zmessage = z3zE1650;
  unit z3zE1651;
  z3zE1651 = UNIT;
  bool z2zE1736;
  z2zE1736 = ztx.zis_create;
  if (z2zE1736) {
    bool z2zE1737;
    z2zE1737 = zupfront.zcreate_target_prestate_empty;
    unit z3zE1663;
    if (z2zE1737) {
      {
        z3zE1663 = zcharge_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_amsterdam_state_new_account);
        if (have_exception) {  goto end_block_exception_2063;  }
      }
    } else {  z3zE1663 = UNIT;  }
    bool z2zE1738;
    z2zE1738 = zis_running(UNIT);
    unit z3zE1662;
    if (z2zE1738) {
      sail_fixed_bytes_32 z2zE1741;
      {
        struct zByteSliceFields z2zE1740;
        {
          struct zByteSliceFields z2zE1739;
          z2zE1739 = ztx.zinput_src;
          z2zE1740 = ztransaction_initcode_slice(z2zE1739);
        }
        z2zE1741 = zcode_db_insert(z2zE1740, zk_fork);
      }
      {
        zframe_code = zcode_db_resolve(z2zE1741);
        if (have_exception) {  goto end_block_exception_2063;  }
      }
      z3zE1662 = UNIT;
    } else {  z3zE1662 = UNIT;  }
    z8zE474 = false;
  } else {
    zcalldata = ztx.zinput_src;
    unit z3zE1654;
    z3zE1654 = UNIT;
    bool z2zE1745;
    {
      bool z2zE1744;
      {
        sail_u256 z2zE1742;
        z2zE1742 = ztx.zvalue;
        z2zE1744 = zword_nonzzero(z2zE1742);
      }
      bool z3zE1652;
      if (z2zE1744) {
        sail_fixed_bytes_20 z2zE1743;
        z2zE1743 = ztx.zrecipient;
        {
          z3zE1652 = zk_account_is_empty(z2zE1743);
          if (have_exception) {  goto end_block_exception_2063;  }
        }
      } else {  z3zE1652 = false;  }
      z2zE1745 = z3zE1652;
    }
    unit z3zE1653;
    if (z2zE1745) {
      {
        z3zE1653 = zcharge_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zG_amsterdam_state_new_account);
        if (have_exception) {  goto end_block_exception_2063;  }
      }
    } else {  z3zE1653 = UNIT;  }
    bool zdelegated;
    zdelegated = false;
    sail_fixed_bytes_20 zdelegate;
    zdelegate = zZERO_ADDRESS;
    bool z2zE1746;
    z2zE1746 = zis_running(UNIT);
    unit z3zE1657;
    if (z2zE1746) {
      struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1748;
      {
        sail_fixed_bytes_20 z2zE1747;
        z2zE1747 = ztx.zrecipient;
        {
          z2zE1748 = zk_deleg_target(z2zE1747);
          if (have_exception) {  goto end_block_exception_2063;  }
        }
      }
      unit z3zE1658;
      {
        bool zis_delegated;
        zis_delegated = z2zE1748.ztup0;
        sail_fixed_bytes_20 ztarget;
        ztarget = z2zE1748.ztup1;
        zdelegated = zis_delegated;
        unit z3zE1660;
        z3zE1660 = UNIT;
        zdelegate = ztarget;
        unit z3zE1659;
        z3zE1659 = UNIT;
        if (zdelegated) {
          bool zwarm;
          zwarm = zk_access_account(zdelegate);
          uint64_t z2zE1749;
          z2zE1749 = zaccount_cost(zwarm);
          {
            z3zE1658 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE1749);
            if (have_exception) {  goto end_block_exception_2063;  }
          }
        } else {  z3zE1658 = UNIT;  }
        goto finish_match_2060;
      }
    case_2061: ;
      sail_match_failure("prepare_amsterdam_transaction_dispatch");
    finish_match_2060: ;
      z3zE1657 = z3zE1658;
    } else {  z3zE1657 = UNIT;  }
    bool z2zE1750;
    z2zE1750 = zis_running(UNIT);
    unit z3zE1655;
    if (z2zE1750) {
      unit z3zE1656;
      if (zdelegated) {
        zmessage.zcode_address = zdelegate;
        z3zE1656 = UNIT;
      } else {  z3zE1656 = UNIT;  }
      sail_fixed_bytes_20 z2zE1751;
      z2zE1751 = ztx.zrecipient;
      {
        zframe_code = zexecutable_code(z2zE1751, zdelegated, zdelegate);
        if (have_exception) {  goto end_block_exception_2063;  }
      }
      z3zE1655 = UNIT;
    } else {  z3zE1655 = UNIT;  }
    z8zE474 = zdelegated;
  }
end_function_2062: ;
  return z8zE474;
end_block_exception_2063: ;

  return false;
}

unit zrun_create_transaction_frame(struct zTransaction ztx, sail_fixed_bytes_20 zsender, uint64_t znonce_before)
{
  unit z8zE475;
  sail_fixed_bytes_20 znew_addr;
  znew_addr = zk_create_addr(zsender, znonce_before);
  bool z2zE1713;
  z2zE1713 = zk_access_account(znew_addr);
  unit z3zE1632;
  {
    bool z2zE1714;
    {
      z2zE1714 = zk_account_occupied(znew_addr);
      if (have_exception) {  goto end_block_exception_2059;  }
    }
    if (z2zE1714) {
      {
        z3zE1632 = zexc_halt(zAddressCollision);
        if (have_exception) {  goto end_block_exception_2059;  }
      }
    } else {
      unit z3zE1640;
      {
        z3zE1640 = zk_mark_created(znew_addr);
        if (have_exception) {  goto end_block_exception_2059;  }
      }
      unit z3zE1639;
      {
        z3zE1639 = zk_clear_storage(znew_addr);
        if (have_exception) {  goto end_block_exception_2059;  }
      }
      unit z3zE1638;
      {
        z3zE1638 = zk_bump_nonce(znew_addr);
        if (have_exception) {  goto end_block_exception_2059;  }
      }
      bool z2zE1716;
      {
        sail_u256 z2zE1715;
        z2zE1715 = ztx.zvalue;
        z2zE1716 = zword_nonzzero(z2zE1715);
      }
      unit z3zE1637;
      if (z2zE1716) {
        sail_u256 z2zE1717;
        z2zE1717 = ztx.zvalue;
        {
          z3zE1637 = zk_transfer(zsender, znew_addr, z2zE1717);
          if (have_exception) {  goto end_block_exception_2059;  }
        }
      } else {  z3zE1637 = UNIT;  }
      bool z2zE1718;
      z2zE1718 = zfork_lt(zk_fork, zAmsterdam);
      unit z3zE1634;
      if (z2zE1718) {
        sail_u256 z2zE1719;
        z2zE1719 = ztx.zvalue;
        struct zMessage z3zE1635;
        z3zE1635.zaddress = znew_addr;
        z3zE1635.zcaller = zsender;
        z3zE1635.zcode_address = znew_addr;
        z3zE1635.zdepth = UINT64_C(0);
        z3zE1635.zis_static = false;
        z3zE1635.zstate_gas_reservoir = zstate_gas_remaining;
        z3zE1635.zvalue = z2zE1719;
        zmessage = z3zE1635;
        unit z3zE1636;
        z3zE1636 = UNIT;
        sail_fixed_bytes_32 z2zE1722;
        {
          struct zByteSliceFields z2zE1721;
          {
            struct zByteSliceFields z2zE1720;
            z2zE1720 = ztx.zinput_src;
            z2zE1721 = ztransaction_initcode_slice(z2zE1720);
          }
          z2zE1722 = zcode_db_insert(z2zE1721, zk_fork);
        }
        {
          zframe_code = zcode_db_resolve(z2zE1722);
          if (have_exception) {  goto end_block_exception_2059;  }
        }
        z3zE1634 = UNIT;
      } else {  z3zE1634 = UNIT;  }
      struct zByteSliceFields zdeployed_code;
      {
        zdeployed_code = zinterpret(UNIT);
        if (have_exception) {  goto end_block_exception_2059;  }
      }
      bool z2zE1723;
      z2zE1723 = zframe_succeeded(UNIT);
      if (z2zE1723) {
        uint64_t zdep_len;
        zdep_len = zdeployed_code.zlen;
        uint64_t zdeployed_length;
        zdeployed_length = zdep_len;
        bool z2zE1728;
        {
          bool z2zE1727;
          z2zE1727 = zdeployed_code_sizze_allowedzIreprzGU64zCozKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zdeployed_length);
          bool z3zE1643;
          if (z2zE1727) {
            bool z2zE1726;
            z2zE1726 = zfork_lt(zk_fork, zLondon);
            bool z3zE1642;
            if (z2zE1726) {  z3zE1642 = true;  } else {
              bool z2zE1725;
              z2zE1725 = (zdeployed_length == UINT64_C(0));
              bool z3zE1641;
              if (z2zE1725) {  z3zE1641 = true;  } else {
                uint64_t z2zE1724;
                z2zE1724 = zslice_byte(zdeployed_code, UINT64_C(0));
                z3zE1641 = (z2zE1724 != UINT64_C(0xEF));
              }
              z3zE1642 = z3zE1641;
            }
            z3zE1643 = z3zE1642;
          } else {  z3zE1643 = false;  }
          z2zE1728 = z3zE1643;
        }
        if (z2zE1728) {
          struct zoptionzIU64zK z2zE1729;
          CREATE(zoptionzIU64zK)(&z2zE1729);
          zcode_deployment_execution_cost(&z2zE1729, zdep_len, zgas_remaining);
          unit z3zE1644;
          {
            if (z2zE1729.kind != Kind_zSomezIU64zK) goto case_2057;
            uint64_t zexecution_deposit;
            zexecution_deposit = z2zE1729.variants.zSomezIU64zK;
            {
              zgas_remaining = zgas_sub_or_oog(zgas_remaining, zexecution_deposit);
              if (have_exception) {
                KILL(zoptionzIU64zK)(&z2zE1729);
                goto end_block_exception_2059;
              }
            }
            unit z3zE1648;
            z3zE1648 = UNIT;
            uint64_t z2zE1730;
            {
              z2zE1730 = zcode_deployment_state_cost(zdep_len);
              if (have_exception) {
                KILL(zoptionzIU64zK)(&z2zE1729);
                goto end_block_exception_2059;
              }
            }
            unit z3zE1647;
            {
              z3zE1647 = zcharge_deployment_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE1730);
              if (have_exception) {
                KILL(zoptionzIU64zK)(&z2zE1729);
                goto end_block_exception_2059;
              }
            }
            bool z2zE1731;
            z2zE1731 = zframe_succeeded(UNIT);
            if (z2zE1731) {
              struct zByteSliceFields z2zE1732;
              z2zE1732 = zvalidated_code_slice(zdeployed_code);
              {
                z3zE1644 = zk_deploy_code(znew_addr, z2zE1732);
                if (have_exception) {
                  KILL(zoptionzIU64zK)(&z2zE1729);
                  goto end_block_exception_2059;
                }
              }
            } else {  z3zE1644 = UNIT;  }
            goto finish_match_2055;
          }
        case_2057: ;
          {
            /* complete */
            bool z2zE1733;
            z2zE1733 = zfork_lt(zk_fork, zHomestead);
            if (z2zE1733) {
              zgas_remaining = zGAS_ZERO;
              unit z3zE1646;
              z3zE1646 = UNIT;
              {
                z3zE1644 = zk_deploy_code(znew_addr, zEMPTY_CODE_SLICE);
                if (have_exception) {
                  KILL(zoptionzIU64zK)(&z2zE1729);
                  goto end_block_exception_2059;
                }
              }
            } else {
              {
                z3zE1644 = zexc_halt(zOutOfGas);
                if (have_exception) {
                  KILL(zoptionzIU64zK)(&z2zE1729);
                  goto end_block_exception_2059;
                }
              }
            }
            goto finish_match_2055;
          }
        case_2056: ;
        finish_match_2055: ;
          z3zE1632 = z3zE1644;
          KILL(zoptionzIU64zK)(&z2zE1729);
        } else {
          {
            z3zE1632 = zexc_halt(zOutOfGas);
            if (have_exception) {  goto end_block_exception_2059;  }
          }
        }
      } else {  z3zE1632 = UNIT;  }
    }
    goto finish_match_2053;
  }
case_2054: ;
  sail_match_failure("run_create_transaction_frame");
finish_match_2053: ;
  z8zE475 = z3zE1632;
end_function_2058: ;
  return z8zE475;
end_block_exception_2059: ;

  return UNIT;
}

unit zrun_call_transaction_frame(struct zTransaction ztx, sail_fixed_bytes_20 zsender, bool zdelegated)
{
  unit z8zE476;
  struct zAccount z2zE1688;
  {
    sail_fixed_bytes_20 z2zE1687;
    z2zE1687 = ztx.zrecipient;
    {
      z2zE1688 = zk_aload(z2zE1687);
      if (have_exception) {  goto end_block_exception_2052;  }
    }
  }
  unit z3zE1611;
  {
    bool z2zE1690;
    {
      sail_u256 z2zE1689;
      z2zE1689 = ztx.zvalue;
      z2zE1690 = zword_nonzzero(z2zE1689);
    }
    unit z3zE1612;
    if (z2zE1690) {
      sail_fixed_bytes_20 z2zE1691;
      z2zE1691 = ztx.zrecipient;
      sail_u256 z2zE1692;
      z2zE1692 = ztx.zvalue;
      {
        z3zE1612 = zk_transfer(zsender, z2zE1691, z2zE1692);
        if (have_exception) {  goto end_block_exception_2052;  }
      }
    } else {  z3zE1612 = UNIT;  }
    uint64_t zselected_precompile;
    {
      sail_fixed_bytes_20 z2zE1712;
      z2zE1712 = ztx.zrecipient;
      zselected_precompile = zprecompile_number(z2zE1712);
    }
    bool z2zE1694;
    {
      bool z2zE1693;
      z2zE1693 = not(zdelegated);
      bool z3zE1613;
      if (z2zE1693) {  z3zE1613 = (zselected_precompile != UINT64_C(0));  } else {  z3zE1613 = false;  }
      z2zE1694 = z3zE1613;
    }
    if (z2zE1694) {
      uint64_t znumber;
      znumber = zselected_precompile;
      struct zoptionzIU64zK z2zE1696;
      CREATE(zoptionzIU64zK)(&z2zE1696);
      {
        struct zByteSliceFields z2zE1695;
        z2zE1695 = ztx.zinput_src;
        zprecompile_gas(&z2zE1696, znumber, z2zE1695, zgas_remaining);
      }
      unit z3zE1628;
      {
        if (z2zE1696.kind != Kind_zSomezIU64zK) goto case_2050;
        uint64_t zused;
        zused = z2zE1696.variants.zSomezIU64zK;
        struct zPrecompileResult zresult;
        {
          struct zByteSliceFields z2zE1701;
          z2zE1701 = ztx.zinput_src;
          zresult = zrun_precompile_slice(znumber, z2zE1701);
        }
        bool z2zE1697;
        z2zE1697 = zresult.zsuccess;
        if (z2zE1697) {
          {
            zgas_remaining = zgas_sub_or_oog(zgas_remaining, zused);
            if (have_exception) {
              KILL(zoptionzIU64zK)(&z2zE1696);
              goto end_block_exception_2052;
            }
          }
          unit z3zE1631;
          z3zE1631 = UNIT;
          bool z2zE1698;
          z2zE1698 = zis_running(UNIT);
          if (z2zE1698) {
            struct zHaltKind z2zE1700;
            CREATE(zHaltKind)(&z2zE1700);
            {
              struct zByteSliceFields z2zE1699;
              z2zE1699 = zresult.zoutput;
              zHaltReturn(&z2zE1700, z2zE1699);
            }
            zHalted(&zframe_status, z2zE1700);
            z3zE1628 = UNIT;
            KILL(zHaltKind)(&z2zE1700);
          } else {  z3zE1628 = UNIT;  }
        } else {
          {
            z3zE1628 = zexc_halt(zOutOfGas);
            if (have_exception) {
              KILL(zoptionzIU64zK)(&z2zE1696);
              goto end_block_exception_2052;
            }
          }
        }
        goto finish_match_2048;
      }
    case_2050: ;
      {
        {
          z3zE1628 = zexc_halt(zOutOfGas);
          if (have_exception) {
            KILL(zoptionzIU64zK)(&z2zE1696);
            goto end_block_exception_2052;
          }
        }
        goto finish_match_2048;
      }
    case_2049: ;
    finish_match_2048: ;
      z3zE1611 = z3zE1628;
      KILL(zoptionzIU64zK)(&z2zE1696);
    } else {
      bool z2zE1702;
      z2zE1702 = zfork_lt(zk_fork, zAmsterdam);
      unit z3zE1615;
      if (z2zE1702) {
        zcalldata = ztx.zinput_src;
        unit z3zE1618;
        z3zE1618 = UNIT;
        sail_fixed_bytes_20 z2zE1703;
        z2zE1703 = ztx.zrecipient;
        sail_fixed_bytes_20 z2zE1704;
        z2zE1704 = ztx.zrecipient;
        sail_u256 z2zE1705;
        z2zE1705 = ztx.zvalue;
        struct zMessage z3zE1616;
        z3zE1616.zaddress = z2zE1703;
        z3zE1616.zcaller = zsender;
        z3zE1616.zcode_address = z2zE1704;
        z3zE1616.zdepth = UINT64_C(0);
        z3zE1616.zis_static = false;
        z3zE1616.zstate_gas_reservoir = zstate_gas_remaining;
        z3zE1616.zvalue = z2zE1705;
        zmessage = z3zE1616;
        unit z3zE1617;
        z3zE1617 = UNIT;
        struct ztuple_z8z5boolzCz0z5structz0zz__sail_c_repr_fixed_bytesz820z9z9 z2zE1707;
        {
          sail_fixed_bytes_20 z2zE1706;
          z2zE1706 = ztx.zrecipient;
          {
            z2zE1707 = zk_deleg_target(z2zE1706);
            if (have_exception) {  goto end_block_exception_2052;  }
          }
        }
        unit z3zE1619;
        {
          bool ztx_deleg;
          ztx_deleg = z2zE1707.ztup0;
          sail_fixed_bytes_20 ztx_dtgt;
          ztx_dtgt = z2zE1707.ztup1;
          unit z3zE1620;
          if (ztx_deleg) {
            bool z2zE1708;
            z2zE1708 = zk_access_account(ztx_dtgt);
            unit z3zE1621;
            {
              struct zAccount z2zE1709;
              {
                z2zE1709 = zk_aload(ztx_dtgt);
                if (have_exception) {  goto end_block_exception_2052;  }
              }
              unit z3zE1622;
              {
                z3zE1622 = UNIT;
                goto finish_match_2044;
              }
            case_2045: ;
              sail_match_failure("run_call_transaction_frame");
            finish_match_2044: ;
              z3zE1621 = z3zE1622;
              goto finish_match_2042;
            }
          case_2043: ;
            sail_match_failure("run_call_transaction_frame");
          finish_match_2042: ;
            z3zE1620 = z3zE1621;
          } else {  z3zE1620 = UNIT;  }
          sail_fixed_bytes_20 z2zE1710;
          z2zE1710 = ztx.zrecipient;
          {
            zframe_code = zexecutable_code(z2zE1710, ztx_deleg, ztx_dtgt);
            if (have_exception) {  goto end_block_exception_2052;  }
          }
          z3zE1619 = UNIT;
          goto finish_match_2040;
        }
      case_2041: ;
        sail_match_failure("run_call_transaction_frame");
      finish_match_2040: ;
        z3zE1615 = z3zE1619;
      } else {  z3zE1615 = UNIT;  }
      struct zByteSliceFields z2zE1711;
      {
        z2zE1711 = zinterpret(UNIT);
        if (have_exception) {  goto end_block_exception_2052;  }
      }
      unit z3zE1626;
      {
        z3zE1626 = UNIT;
        goto finish_match_2046;
      }
    case_2047: ;
      sail_match_failure("run_call_transaction_frame");
    finish_match_2046: ;
      z3zE1611 = z3zE1626;
    }
    goto finish_match_2038;
  }
case_2039: ;
  sail_match_failure("run_call_transaction_frame");
finish_match_2038: ;
  z8zE476 = z3zE1611;
end_function_2051: ;
  return z8zE476;
end_block_exception_2052: ;

  return UNIT;
}

struct zTxFrameResult zrun_legacy_transaction_frame(struct zTransaction ztx, struct zTxValidity zv)
{
  struct zTxFrameResult z8zE477;
  uint64_t zcheckpoint;
  zcheckpoint = zk_state_checkpoint(UNIT);
  uint64_t z2zE1677;
  z2zE1677 = zv.zgas_limit;
  uint64_t z2zE1678;
  z2zE1678 = zv.zintrinsic_execution_gas;
  uint64_t z2zE1679;
  z2zE1679 = zv.zintrinsic_state_gas;
  unit z3zE1608;
  {
    z3zE1608 = zenter_transaction_frame(z2zE1677, z2zE1678, z2zE1679);
    if (have_exception) {  goto end_block_exception_2037;  }
  }
  bool z2zE1680;
  z2zE1680 = ztx.zis_create;
  unit z3zE1607;
  if (z2zE1680) {
    sail_fixed_bytes_20 z2zE1681;
    z2zE1681 = zv.zsender;
    uint64_t z2zE1682;
    z2zE1682 = zv.znonce_before;
    {
      z3zE1607 = zrun_create_transaction_frame(ztx, z2zE1681, z2zE1682);
      if (have_exception) {  goto end_block_exception_2037;  }
    }
  } else {
    sail_fixed_bytes_20 z2zE1683;
    z2zE1683 = zv.zsender;
    {
      z3zE1607 = zrun_call_transaction_frame(ztx, z2zE1683, false);
      if (have_exception) {  goto end_block_exception_2037;  }
    }
  }
  bool zsuccess;
  zsuccess = zframe_succeeded(UNIT);
  bool z2zE1684;
  z2zE1684 = not(zsuccess);
  unit z3zE1609;
  if (z2zE1684) {  z3zE1609 = zk_revert(zcheckpoint);  } else {  z3zE1609 = UNIT;  }
  __int128 z2zE1685;
  z2zE1685 = zframe_state_gas_used(UNIT);
  __int128 z2zE1686;
  if (zsuccess) {  z2zE1686 = zframe_refund;  } else {  z2zE1686 = zGAS_REFUND_ZERO;  }
  struct zTxFrameResult z3zE1610;
  z3zE1610.zexecution_gas_remaining = zgas_remaining;
  z3zE1610.zrefund = z2zE1686;
  z3zE1610.zstate_gas_remaining = zstate_gas_remaining;
  z3zE1610.zstate_gas_used = z2zE1685;
  z3zE1610.zsuccess = zsuccess;
  z8zE477 = z3zE1610;
end_function_2036: ;
  return z8zE477;
end_block_exception_2037: ;
  struct zTxFrameResult z8zE995 = { .zexecution_gas_remaining = UINT64_C(0xdeadc0de), .zrefund = ((__int128)INT64_C(0xdeadc0de)), .zstate_gas_remaining = UINT64_C(0xdeadc0de), .zstate_gas_used = ((__int128)INT64_C(0xdeadc0de)), .zsuccess = false };
  return z8zE995;
}

struct zTxFrameResult zrun_amsterdam_transaction_frame(struct zTransaction ztx, struct zTxValidity zv, struct zTxUpfrontResult zupfront)
{
  struct zTxFrameResult z8zE478;
  uint64_t z2zE1655;
  z2zE1655 = zv.zgas_limit;
  uint64_t z2zE1656;
  z2zE1656 = zv.zintrinsic_execution_gas;
  uint64_t z2zE1657;
  z2zE1657 = zv.zintrinsic_state_gas;
  unit z3zE1591;
  {
    z3zE1591 = zenter_transaction_frame(z2zE1655, z2zE1656, z2zE1657);
    if (have_exception) {  goto end_block_exception_2035;  }
  }
  uint64_t zpreparation_checkpoint;
  zpreparation_checkpoint = zk_state_checkpoint(UNIT);
  uint64_t zpreparation_reservoir;
  zpreparation_reservoir = zstate_gas_remaining;
  sail_fixed_bytes_20 zcurrent_target;
  {
    bool z2zE1674;
    z2zE1674 = ztx.zis_create;
    if (z2zE1674) {
      sail_fixed_bytes_20 z2zE1675;
      z2zE1675 = zv.zsender;
      uint64_t z2zE1676;
      z2zE1676 = zv.znonce_before;
      zcurrent_target = zk_create_addr(z2zE1675, z2zE1676);
    } else {  zcurrent_target = ztx.zrecipient;  }
  }
  struct zAmsterdamAuthorizzationState z2zE1662;
  CREATE(zAmsterdamAuthorizzationState)(&z2zE1662);
  {
    zz5listz8z5structz0zzAuthorizzzzationz9 z2zE1659;
    CREATE(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE1659);
    COPY(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE1659, ztx.zauthorizzations);
    sail_fixed_bytes_20 z2zE1660;
    z2zE1660 = zv.zsender;
    bool z2zE1661;
    {
      sail_u256 z2zE1658;
      z2zE1658 = ztx.zvalue;
      z2zE1661 = zword_nonzzero(z2zE1658);
    }
    {
      zprocess_amsterdam_auth_list(&z2zE1662, z2zE1659, z2zE1660, zcurrent_target, z2zE1661, zEMPTY_AMSTERDAM_AUTHORIZATION_STATE);
      if (have_exception) {
        KILL(zAmsterdamAuthorizzationState)(&z2zE1662);
        KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE1659);
        goto end_block_exception_2035;
      }
    }
    KILL(zz5listz8z5structz0zzAuthorizzzzationz9)(&z2zE1659);
  }
  struct zTxFrameResult z3zE1592;
  {
    __int128 zauthorizzation_state_gas;
    zauthorizzation_state_gas = zFRAME_STATE_GAS_DELTA_ZERO;
    bool zdelegated;
    zdelegated = false;
    bool z2zE1663;
    z2zE1663 = zis_running(UNIT);
    unit z3zE1599;
    if (z2zE1663) {
      zauthorizzation_state_gas = zframe_state_gas_used(UNIT);
      unit z3zE1602;
      z3zE1602 = UNIT;
      zmessage.zstate_gas_reservoir = zstate_gas_remaining;
      unit z3zE1601;
      z3zE1601 = UNIT;
      zstate_gas_spilled = zSTATE_GAS_SPILL_ZERO;
      unit z3zE1600;
      z3zE1600 = UNIT;
      {
        zdelegated = zprepare_amsterdam_transaction_dispatch(ztx, zv, zupfront);
        if (have_exception) {
          KILL(zAmsterdamAuthorizzationState)(&z2zE1662);
          goto end_block_exception_2035;
        }
      }
      z3zE1599 = UNIT;
    } else {  z3zE1599 = UNIT;  }
    bool z2zE1665;
    {
      bool z2zE1664;
      z2zE1664 = zis_running(UNIT);
      z2zE1665 = not(z2zE1664);
    }
    unit z3zE1593;
    if (z2zE1665) {
      unit z3zE1597;
      z3zE1597 = zk_revert(zpreparation_checkpoint);
      zmessage.zstate_gas_reservoir = zpreparation_reservoir;
      unit z3zE1596;
      z3zE1596 = UNIT;
      zstate_gas_remaining = zpreparation_reservoir;
      unit z3zE1595;
      z3zE1595 = UNIT;
      zstate_gas_spilled = zSTATE_GAS_SPILL_ZERO;
      unit z3zE1594;
      z3zE1594 = UNIT;
      struct zTxFrameResult z3zE1598;
      z3zE1598.zexecution_gas_remaining = zgas_remaining;
      z3zE1598.zrefund = zGAS_REFUND_ZERO;
      z3zE1598.zstate_gas_remaining = zstate_gas_remaining;
      z3zE1598.zstate_gas_used = zSTATE_GAS_DELTA_ZERO;
      z3zE1598.zsuccess = false;
      z8zE478 = z3zE1598;
      goto cleanup_2033;
      /* unreachable after return */
      goto end_cleanup_2034;
    cleanup_2033: ;
      KILL(zAmsterdamAuthorizzationState)(&z2zE1662);
      goto end_function_2032;
    end_cleanup_2034: ;
    } else {  z3zE1593 = UNIT;  }
    uint64_t zexecution_checkpoint;
    zexecution_checkpoint = zk_state_checkpoint(UNIT);
    bool z2zE1666;
    z2zE1666 = ztx.zis_create;
    unit z3zE1603;
    if (z2zE1666) {
      sail_fixed_bytes_20 z2zE1667;
      z2zE1667 = zv.zsender;
      uint64_t z2zE1668;
      z2zE1668 = zv.znonce_before;
      {
        z3zE1603 = zrun_create_transaction_frame(ztx, z2zE1667, z2zE1668);
        if (have_exception) {
          KILL(zAmsterdamAuthorizzationState)(&z2zE1662);
          goto end_block_exception_2035;
        }
      }
    } else {
      sail_fixed_bytes_20 z2zE1669;
      z2zE1669 = zv.zsender;
      {
        z3zE1603 = zrun_call_transaction_frame(ztx, z2zE1669, zdelegated);
        if (have_exception) {
          KILL(zAmsterdamAuthorizzationState)(&z2zE1662);
          goto end_block_exception_2035;
        }
      }
    }
    bool zsuccess;
    zsuccess = zframe_succeeded(UNIT);
    bool z2zE1670;
    z2zE1670 = not(zsuccess);
    unit z3zE1604;
    if (z2zE1670) {  z3zE1604 = zk_revert(zexecution_checkpoint);  } else {  z3zE1604 = UNIT;  }
    __int128 z2zE1672;
    {
      __int128 z2zE1671;
      z2zE1671 = zframe_state_gas_used(UNIT);
      {    z2zE1672 = (zauthorizzation_state_gas + z2zE1671);
      }
    }
    __int128 z2zE1673;
    if (zsuccess) {  z2zE1673 = zframe_refund;  } else {  z2zE1673 = zGAS_REFUND_ZERO;  }
    struct zTxFrameResult z3zE1605;
    z3zE1605.zexecution_gas_remaining = zgas_remaining;
    z3zE1605.zrefund = z2zE1673;
    z3zE1605.zstate_gas_remaining = zstate_gas_remaining;
    z3zE1605.zstate_gas_used = z2zE1672;
    z3zE1605.zsuccess = zsuccess;
    z3zE1592 = z3zE1605;
    goto finish_match_2030;
  }
case_2031: ;
  sail_match_failure("run_amsterdam_transaction_frame");
finish_match_2030: ;
  z8zE478 = z3zE1592;
  KILL(zAmsterdamAuthorizzationState)(&z2zE1662);
end_function_2032: ;
  return z8zE478;
end_block_exception_2035: ;
  struct zTxFrameResult z8zE996 = { .zexecution_gas_remaining = UINT64_C(0xdeadc0de), .zrefund = ((__int128)INT64_C(0xdeadc0de)), .zstate_gas_remaining = UINT64_C(0xdeadc0de), .zstate_gas_used = ((__int128)INT64_C(0xdeadc0de)), .zsuccess = false };
  return z8zE996;
}

struct zTxFrameResult zrun_transaction_frame(struct zTransaction ztx, struct zTxValidity zv, struct zTxUpfrontResult zupfront)
{
  struct zTxFrameResult z8zE479;
  bool z2zE1654;
  z2zE1654 = zfork_gteq(zk_fork, zAmsterdam);
  if (z2zE1654) {
    {
      z8zE479 = zrun_amsterdam_transaction_frame(ztx, zv, zupfront);
      if (have_exception) {  goto end_block_exception_2029;  }
    }
  } else {
    {
      z8zE479 = zrun_legacy_transaction_frame(ztx, zv);
      if (have_exception) {  goto end_block_exception_2029;  }
    }
  }
end_function_2028: ;
  return z8zE479;
end_block_exception_2029: ;
  struct zTxFrameResult z8zE997 = { .zexecution_gas_remaining = UINT64_C(0xdeadc0de), .zrefund = ((__int128)INT64_C(0xdeadc0de)), .zstate_gas_remaining = UINT64_C(0xdeadc0de), .zstate_gas_used = ((__int128)INT64_C(0xdeadc0de)), .zsuccess = false };
  return z8zE997;
}

uint64_t zadmitted_transaction_state_gas(__int128 zvalue)
{
  uint64_t z8zE480;
  bool z2zE1648;
  z2zE1648 = (!(UINT64_C(0) < zvalue));
  if (z2zE1648) {  z8zE480 = zGAS_ZERO;  } else {
    bool z2zE1649;
    z2zE1649 = (!(UINT64_C(18446744073709551615) < zvalue));
    if (z2zE1649) {  z8zE480 = (uint64_t)(zvalue);  } else {
      struct zexception z2zE1650;
      CREATE(zexception)(&z2zE1650);
      zInvalidBlock(&z2zE1650, zExecutionInvalid);
      COPY(zexception)(current_exception, z2zE1650);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/evm/transaction.sail:1049.12-1049.48");
      KILL(zexception)(&z2zE1650);
      goto end_block_exception_2025;
      /* unreachable after throw */
      KILL(zexception)(&z2zE1650);
    }
  }
end_function_2024: ;
  return z8zE480;
end_block_exception_2025: ;

  return UINT64_C(0xdeadc0de);
}

void zsettle_transaction(struct zReceipt *z8zE481, struct zTransaction ztx, struct zTxValidity zv, __int128 zauthorizzation_refund, struct zTxFrameResult zfr)
{
  uint64_t zgas_left0;
  {
    uint64_t z2zE1646;
    z2zE1646 = zfr.zexecution_gas_remaining;
    uint64_t z2zE1647;
    z2zE1647 = zfr.zstate_gas_remaining;
    {
      zgas_left0 = zvalidated_gas_add(z2zE1646, z2zE1647);
      if (have_exception) {  goto end_block_exception_2023;  }
    }
  }
  uint64_t zgas_used0;
  {
    uint64_t z2zE1645;
    z2zE1645 = zv.zgas_limit;
    {
      zgas_used0 = zvalidated_gas_subzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE1645, zgas_left0);
      if (have_exception) {  goto end_block_exception_2023;  }
    }
  }
  uint64_t zrefund_quotient;
  {
    bool z2zE1644;
    z2zE1644 = zfork_gteq(zk_fork, zLondon);
    if (z2zE1644) {  zrefund_quotient = UINT64_C(5);  } else {  zrefund_quotient = UINT64_C(2);  }
  }
  uint64_t zrefund_cap;
  zrefund_cap = (zgas_used0 / zrefund_quotient);
  __int128 ztotal_refund;
  {
    __int128 z2zE1643;
    z2zE1643 = zfr.zrefund;
    {
      ztotal_refund = zvalidated_refund_add(zauthorizzation_refund, z2zE1643);
      if (have_exception) {  goto end_block_exception_2023;  }
    }
  }
  uint64_t zrefund;
  zrefund = zcapped_transaction_refundzIreprzGI128zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(ztotal_refund, zrefund_cap);
  uint64_t zgas_left1;
  {
    zgas_left1 = zvalidated_gas_add(zgas_left0, zrefund);
    if (have_exception) {  goto end_block_exception_2023;  }
  }
  uint64_t zgas_used1;
  {
    uint64_t z2zE1642;
    z2zE1642 = zv.zgas_limit;
    {
      zgas_used1 = zvalidated_gas_subzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE1642, zgas_left1);
      if (have_exception) {  goto end_block_exception_2023;  }
    }
  }
  uint64_t zfloor;
  {
    bool z2zE1639;
    z2zE1639 = zfork_gteq(zk_fork, zPrague);
    if (z2zE1639) {
      uint64_t zfloor_cost;
      zfloor_cost = zv.zcalldata_floor;
      uint64_t ztx_limit;
      ztx_limit = zv.zgas_limit;
      bool z2zE1640;
      z2zE1640 = (!(ztx_limit < zfloor_cost));
      if (z2zE1640) {  zfloor = zfloor_cost;  } else {
        struct zexception z2zE1641;
        CREATE(zexception)(&z2zE1641);
        zInvalidBlock(&z2zE1641, zExecutionInvalid);
        COPY(zexception)(current_exception, z2zE1641);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/evm/transaction.sail:1084.16-1084.52");
        KILL(zexception)(&z2zE1641);
        goto end_block_exception_2023;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1641);
      }
    } else {  zfloor = UINT64_C(0);  }
  }
  uint64_t zgas_used;
  {
    bool z2zE1638;
    z2zE1638 = (zgas_used1 < zfloor);
    if (z2zE1638) {  zgas_used = zfloor;  } else {  zgas_used = zgas_used1;  }
  }
  uint64_t zgas_left;
  {
    uint64_t z2zE1637;
    z2zE1637 = zv.zgas_limit;
    {
      zgas_left = zvalidated_gas_subzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE1637, zgas_used);
      if (have_exception) {  goto end_block_exception_2023;  }
    }
  }
  __int128 zraw_state_gas;
  {
    __int128 z2zE1635;
    z2zE1635 = zfr.zstate_gas_used;
    uint64_t z2zE1636;
    z2zE1636 = zv.zintrinsic_state_gas;
    {
      __int128 z3zE1585;
      z3zE1585 = (__int128)(z2zE1636);
      zraw_state_gas = (z2zE1635 + z3zE1585);
    }
  }
  uint64_t ztx_state_gas;
  {
    ztx_state_gas = zadmitted_transaction_state_gas(zraw_state_gas);
    if (have_exception) {  goto end_block_exception_2023;  }
  }
  uint64_t zexecution_before_floor;
  zexecution_before_floor = zGAS_ZERO;
  bool z2zE1622;
  z2zE1622 = (!(zgas_used0 < ztx_state_gas));
  unit z3zE1586;
  if (z2zE1622) {
    uint64_t zreduced_execution_gas;
    {
      zreduced_execution_gas = zvalidated_gas_subzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zgas_used0, ztx_state_gas);
      if (have_exception) {  goto end_block_exception_2023;  }
    }
    zexecution_before_floor = zreduced_execution_gas;
    z3zE1586 = UNIT;
  } else {  z3zE1586 = UNIT;  }
  uint64_t zexecution_gas;
  {
    bool z2zE1633;
    z2zE1633 = zfork_gteq(zk_fork, zAmsterdam);
    if (z2zE1633) {
      bool z2zE1634;
      z2zE1634 = (zexecution_before_floor < zfloor);
      if (z2zE1634) {  zexecution_gas = zfloor;  } else {  zexecution_gas = zexecution_before_floor;  }
    } else {  zexecution_gas = zgas_used;  }
  }
  uint64_t zstate_gas;
  {
    bool z2zE1632;
    z2zE1632 = zfork_gteq(zk_fork, zAmsterdam);
    if (z2zE1632) {  zstate_gas = ztx_state_gas;  } else {  zstate_gas = zGAS_ZERO;  }
  }
  sail_fixed_bytes_20 z2zE1624;
  z2zE1624 = zv.zsender;
  sail_u256 z2zE1625;
  {
    sail_u256 z2zE1623;
    z2zE1623 = zv.zgas_price;
    {
      z2zE1625 = zvalidated_word_productzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE1623, zgas_left);
      if (have_exception) {  goto end_block_exception_2023;  }
    }
  }
  unit z3zE1589;
  {
    z3zE1589 = zk_add_balance(z2zE1624, z2zE1625);
    if (have_exception) {  goto end_block_exception_2023;  }
  }
  sail_fixed_bytes_20 z2zE1627;
  z2zE1627 = zk_coinbase(UNIT);
  sail_u256 z2zE1628;
  {
    sail_u256 z2zE1626;
    z2zE1626 = zv.zpriority_fee;
    {
      z2zE1628 = zvalidated_word_productzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE1626, zgas_used);
      if (have_exception) {  goto end_block_exception_2023;  }
    }
  }
  unit z3zE1588;
  {
    z3zE1588 = zk_add_balance(z2zE1627, z2zE1628);
    if (have_exception) {  goto end_block_exception_2023;  }
  }
  unit z3zE1587;
  z3zE1587 = zk_tx_merge(UNIT);
  enum zTxType z2zE1629;
  z2zE1629 = ztx.ztx_type;
  bool z2zE1630;
  z2zE1630 = zfr.zsuccess;
  zz5listz8z5structz0zzLogEntryz9 z2zE1631;
  CREATE(zz5listz8z5structz0zzLogEntryz9)(&z2zE1631);
  logs_read_all(&z2zE1631, UNIT);
  struct zReceipt z3zE1590;
  CREATE(zReceipt)(&z3zE1590);
  z3zE1590.zexecution_gas = zexecution_gas;
  z3zE1590.zgas_used = zgas_used;
  COPY(zz5listz8z5structz0zzLogEntryz9)(&((&z3zE1590)->zlogs), z2zE1631);
  z3zE1590.zstate_gas = zstate_gas;
  z3zE1590.zsuccess = z2zE1630;
  z3zE1590.ztx_type = z2zE1629;
  COPY(zReceipt)((*(&z8zE481)), z3zE1590);
  KILL(zReceipt)(&z3zE1590);
  KILL(zz5listz8z5structz0zzLogEntryz9)(&z2zE1631);
end_function_2022: ;
  goto end_function_4062;
end_block_exception_2023: ;
  goto end_function_4062;
end_function_4062: ;
}

void zprocess_transaction(struct zReceipt *z8zE482, struct zTransaction ztx)
{
  unit z3zE1575;
  z3zE1575 = zcycle_scope_start(zSCOPE_TX_RESET);
  unit z3zE1574;
  z3zE1574 = zk_tx_reset(UNIT);
  unit z3zE1573;
  z3zE1573 = zcycle_scope_end(zSCOPE_TX_RESET);
  unit z3zE1572;
  z3zE1572 = zcycle_scope_start(zSCOPE_TX_VALIDATE);
  struct zTxValidity zvalidity;
  {
    zvalidity = zcheck_transaction_validity(ztx);
    if (have_exception) {  goto end_block_exception_2021;  }
  }
  unit z3zE1579;
  z3zE1579 = zcycle_scope_end(zSCOPE_TX_VALIDATE);
  struct zTxEnv z2zE1620;
  {
    sail_fixed_bytes_20 z2zE1617;
    z2zE1617 = ztx.zsender;
    sail_u256 z2zE1618;
    z2zE1618 = zvalidity.zgas_price;
    struct zBlobHashes z2zE1619;
    z2zE1619 = ztx.zblob_hashes;
    struct zTxEnv z3zE1576;
    z3zE1576.zblob_hashes = z2zE1619;
    z3zE1576.zgas_price = z2zE1618;
    z3zE1576.zorigin = z2zE1617;
    z2zE1620 = z3zE1576;
  }
  unit z3zE1578;
  z3zE1578 = zk_set_tx(z2zE1620);
  unit z3zE1577;
  z3zE1577 = zcycle_scope_start(zSCOPE_TX_UPFRONT);
  struct zTxUpfrontResult zupfront;
  {
    zupfront = zapply_transaction_upfront_effects(ztx, zvalidity);
    if (have_exception) {  goto end_block_exception_2021;  }
  }
  unit z3zE1581;
  z3zE1581 = zcycle_scope_end(zSCOPE_TX_UPFRONT);
  unit z3zE1580;
  z3zE1580 = zcycle_scope_start(zSCOPE_TX_FRAME);
  struct zTxFrameResult zframe_result;
  {
    zframe_result = zrun_transaction_frame(ztx, zvalidity, zupfront);
    if (have_exception) {  goto end_block_exception_2021;  }
  }
  unit z3zE1583;
  z3zE1583 = zcycle_scope_end(zSCOPE_TX_FRAME);
  unit z3zE1582;
  z3zE1582 = zcycle_scope_start(zSCOPE_TX_SETTLE);
  struct zReceipt zreceipt;
  CREATE(zReceipt)(&zreceipt);
  {
    __int128 z2zE1621;
    z2zE1621 = zupfront.zauthorizzation_refund;
    {
      zsettle_transaction(&zreceipt, ztx, zvalidity, z2zE1621, zframe_result);
      if (have_exception) {
        KILL(zReceipt)(&zreceipt);
        goto end_block_exception_2021;
      }
    }
  }
  unit z3zE1584;
  z3zE1584 = zcycle_scope_end(zSCOPE_TX_SETTLE);
  COPY(zReceipt)((*(&z8zE482)), zreceipt);
  KILL(zReceipt)(&zreceipt);
end_function_2020: ;
  goto end_function_4061;
end_block_exception_2021: ;
  goto end_function_4061;
end_function_4061: ;
}

