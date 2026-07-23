#ifndef EVMSAIL_C_UNITY_BUILD
#define EVMSAIL_C_UNITY_BUILD 1
#endif
#include "evm_internal.h"
#include "prelude.c"
#include "primitives/cycle_scopes.c"
#include "host/cycle_scopes_disabled.c"
#include "primitives/quantities.c"
#include "primitives/gas.c"
#include "primitives/bytes.c"
#include "exceptions.c"
#include "evm/halt.c"
#include "primitives/ssz.c"
#include "primitives/code.c"
#include "lib/bytes.c"
#include "host/accelerators.c"
#include "primitives/rlp.c"
#include "primitives/crypto.c"
#include "primitives/fork.c"
#include "primitives/chain_config.c"
#include "primitives/system.c"
#include "primitives/account.c"
#include "primitives/tx.c"
#include "primitives/block.c"
#include "primitives/evm.c"
#include "primitives/stateless_input.c"
#include "host/byte_slice.c"
#include "host/scratch.c"
#include "host/kernel/scratch.c"
#include "host/code.c"
#include "host/nodes.c"
#include "lib/ssz/ssz.c"
#include "lib/rlp/rlp.c"
#include "lib/address.c"
#include "lib/tx.c"
#include "lib/rlp/tx.c"
#include "host/state.c"
#include "host/environment.c"
#include "host/stack.c"
#include "host/memory.c"
#include "host/output.c"
#include "host/kernel/environment.c"
#include "host/kernel/storage.c"
#include "host/kernel/logs.c"
#include "host/kernel/accounts.c"
#include "host/kernel/code.c"
#include "host/kernel/selfdestruct.c"
#include "host/kernel/lifecycle.c"
#include "evm/machine.c"
#include "evm/gas.c"
#include "evm/precompiles.c"
#include "evm/instructions.c"
#include "evm/execute.c"
#include "evm/interpreter.c"
#include "evm/transaction.c"
#include "lib/mpt/primitives.c"
#include "lib/mpt/nodes.c"
#include "lib/mpt/updates.c"
#include "lib/mpt/indexed.c"
#include "lib/mpt/trie.c"
#include "lib/state_trie.c"
#include "host/debug_disabled.c"
#include "lib/ssz/stateless_input.c"
#include "executor/receipts.c"
#include "executor/system_calls.c"
#include "executor/block_access_list.c"
#include "executor/block.c"
#include "executor/payload.c"
#include "executor/stateless.c"
#include "lib/htr.c"
#include "executor/result.c"
#include "main.c"

unit zinitializze_registers(unit z3zE445)
{
  unit z8zE721;
  z8zE721 = UNIT;
end_function_1122: ;
  return z8zE721;
end_block_exception_1123: ;

  return UNIT;
}

sail_u256 zU256zIreprzGR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(sail_u128 zvalue)
{
  sail_u256 z8zE722;
  z8zE722 = u256_of_u128(zvalue);
end_function_3970: ;
  return z8zE722;
end_block_exception_3971: ;

  return u256_zero();
}

sail_u256 zU256zIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zvalue)
{
  sail_u256 z8zE723;
  z8zE723 = u256_of_fbits(zvalue);
end_function_3970: ;
  return z8zE723;
end_block_exception_3971: ;

  return u256_zero();
}

sail_u256 zalu_addzIreprzGR__sail_c_repr_u256zCR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256 za, sail_u128 zb)
{
  sail_u256 z8zE724;
  z8zE724 = zword_add_wordzIreprzGR__sail_c_repr_u256zCR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(za, zb);
end_function_3894: ;
  return z8zE724;
end_block_exception_3895: ;

  return u256_zero();
}

sail_u256 zalu_addzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256 za, uint64_t zb)
{
  sail_u256 z8zE725;
  z8zE725 = zword_add_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(za, zb);
end_function_3894: ;
  return z8zE725;
end_block_exception_3895: ;

  return u256_zero();
}

struct zBalContentCount zbal_accounts_sizzezIreprzGU64zCU64zCRBalContentCountzCRBalContentCountzKzIboundszG14423a7d1e032b2eb688e7c175cd02a1zK(uint64_t zaccount_count, uint64_t zaccount, struct zBalContentCount zresult)
{
  struct zBalContentCount z8zE726;
  bool z2zE529;
  {    z2zE529 = (zaccount < zaccount_count);
  }
  if (z2zE529) {
    struct zBalAccountSizze zaccount_sizze;
    {
      zaccount_sizze = zbal_account_sizze(zaccount);
      if (have_exception) {  goto end_block_exception_1382;  }
    }
    uint64_t znext_length;
    {
      uint64_t z2zE536;
      z2zE536 = zresult.zcontent_len;
      uint64_t z2zE537;
      {
        uint64_t z2zE535;
        z2zE535 = zaccount_sizze.zencoded_len;
        z2zE537 = zbal_rlp_length_to_byte_length(z2zE535);
      }
      {
        znext_length = zbal_rlp_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE536, z2zE537);
        if (have_exception) {  goto end_block_exception_1382;  }
      }
    }
    uint64_t z2zE533;
    {
      sail_u128 z3zE3820;
      z3zE3820 = u128_of_u64(zaccount);
      sail_u128 z3zE3821;
      z3zE3821 = u128_add_u64(z3zE3820, UINT64_C(1));
      z2zE533 = u128_to_u64(z3zE3821);
    }
    struct zBalContentCount z2zE534;
    {
      uint64_t z2zE532;
      {
        uint64_t z2zE530;
        z2zE530 = zresult.zcount;
        uint64_t z2zE531;
        z2zE531 = zaccount_sizze.zitem_count;
        {
          z2zE532 = zbal_count_addzIreprzGU64zCU64zCU64zKzIboundszGd0328719294598864996af0723f2d8e9zK(z2zE530, z2zE531);
          if (have_exception) {  goto end_block_exception_1382;  }
        }
      }
      struct zBalContentCount z3zE915;
      z3zE915.zcontent_len = znext_length;
      z3zE915.zcount = z2zE532;
      z2zE534 = z3zE915;
    }
    {
      z8zE726 = zbal_accounts_sizzezIreprzGU64zCU64zCRBalContentCountzCRBalContentCountzKzIboundszG14423a7d1e032b2eb688e7c175cd02a1zK(zaccount_count, z2zE533, z2zE534);
      if (have_exception) {  goto end_block_exception_1382;  }
    }
  } else {  z8zE726 = zresult;  }
end_function_1381: ;
  return z8zE726;
end_block_exception_1382: ;
  struct zBalContentCount z8zE1152 = { .zcontent_len = UINT64_C(0xdeadc0de), .zcount = UINT64_C(0xdeadc0de) };
  return z8zE1152;
}

struct zBalContentCount zbal_accounts_sizzezIreprzGU64zCU64zCRBalContentCountzCRBalContentCountzKzIboundszGa3760b966d2df2175e3b9bd94efd7f00zK(uint64_t zaccount_count, uint64_t zaccount, struct zBalContentCount zresult)
{
  struct zBalContentCount z8zE727;
  bool z2zE529;
  {    z2zE529 = (zaccount < zaccount_count);
  }
  if (z2zE529) {
    struct zBalAccountSizze zaccount_sizze;
    {
      zaccount_sizze = zbal_account_sizze(zaccount);
      if (have_exception) {  goto end_block_exception_1382;  }
    }
    uint64_t znext_length;
    {
      uint64_t z2zE536;
      z2zE536 = zresult.zcontent_len;
      uint64_t z2zE537;
      {
        uint64_t z2zE535;
        z2zE535 = zaccount_sizze.zencoded_len;
        z2zE537 = zbal_rlp_length_to_byte_length(z2zE535);
      }
      {
        znext_length = zbal_rlp_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE536, z2zE537);
        if (have_exception) {  goto end_block_exception_1382;  }
      }
    }
    uint64_t z2zE533;
    {    z2zE533 = (UINT64_C(1) + zaccount);
    }
    struct zBalContentCount z2zE534;
    {
      uint64_t z2zE532;
      {
        uint64_t z2zE530;
        z2zE530 = zresult.zcount;
        uint64_t z2zE531;
        z2zE531 = zaccount_sizze.zitem_count;
        {
          z2zE532 = zbal_count_addzIreprzGU64zCU64zCU64zKzIboundszGd0328719294598864996af0723f2d8e9zK(z2zE530, z2zE531);
          if (have_exception) {  goto end_block_exception_1382;  }
        }
      }
      struct zBalContentCount z3zE915;
      z3zE915.zcontent_len = znext_length;
      z3zE915.zcount = z2zE532;
      z2zE534 = z3zE915;
    }
    {
      z8zE727 = zbal_accounts_sizzezIreprzGU64zCU64zCRBalContentCountzCRBalContentCountzKzIboundszGa3760b966d2df2175e3b9bd94efd7f00zK(zaccount_count, z2zE533, z2zE534);
      if (have_exception) {  goto end_block_exception_1382;  }
    }
  } else {  z8zE727 = zresult;  }
end_function_1381: ;
  return z8zE727;
end_block_exception_1382: ;
  struct zBalContentCount z8zE1153 = { .zcontent_len = UINT64_C(0xdeadc0de), .zcount = UINT64_C(0xdeadc0de) };
  return z8zE1153;
}

uint64_t zbal_bounded_byte_length_addzIreprzGU64zCR__sail_c_repr_u128zCU64zKzIboundszG69b6fbdbe405b82ee0c43387d0dda91azK(uint64_t zleft, sail_u128 zright)
{
  uint64_t z8zE728;
  uint64_t z2zE700;
  {
    uint64_t z2zE699;
    {
      z2zE699 = zbal_rlp_length_from_byte_lengthzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zleft);
      if (have_exception) {  goto end_block_exception_1470;  }
    }
    {
      z2zE700 = zbal_rlp_length_addzIreprzGU64zCR__sail_c_repr_u128zCU64zKzIboundszGebfa7455a07eda44e5082c02fe62be56zK(z2zE699, zright);
      if (have_exception) {  goto end_block_exception_1470;  }
    }
  }
  z8zE728 = zbal_rlp_length_to_byte_length(z2zE700);
end_function_1469: ;
  return z8zE728;
end_block_exception_1470: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zbal_bounded_byte_length_addzIreprzGU64zCU64zCU64zKzIboundszGa3b2b826adddb1d18c77e659efbf6001zK(uint64_t zleft, uint64_t zright)
{
  uint64_t z8zE729;
  uint64_t z2zE700;
  {
    uint64_t z2zE699;
    {
      z2zE699 = zbal_rlp_length_from_byte_lengthzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zleft);
      if (have_exception) {  goto end_block_exception_1470;  }
    }
    {
      z2zE700 = zbal_rlp_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE699, zright);
      if (have_exception) {  goto end_block_exception_1470;  }
    }
  }
  z8zE729 = zbal_rlp_length_to_byte_length(z2zE700);
end_function_1469: ;
  return z8zE729;
end_block_exception_1470: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zbal_bounded_list_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zcontent_len)
{
  uint64_t z8zE730;
  uint64_t z2zE698;
  {
    uint64_t z2zE697;
    {
      z2zE697 = zbal_rlp_length_from_byte_lengthzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zcontent_len);
      if (have_exception) {  goto end_block_exception_1466;  }
    }
    {
      z2zE698 = zbal_rlp_list_sizze(z2zE697);
      if (have_exception) {  goto end_block_exception_1466;  }
    }
  }
  z8zE730 = zbal_rlp_length_to_byte_length(z2zE698);
end_function_1465: ;
  return z8zE730;
end_block_exception_1466: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zbal_count_addzIreprzGU64zCU64zCU64zKzIboundszG43fd78b15b770f2258241a12de1a7119zK(uint64_t za, uint64_t zb)
{
  uint64_t z8zE731;
  bool z2zE695;
  {
    bool z2zE694;
    z2zE694 = true;
    bool z3zE962;
    if (z2zE694) {
      uint64_t z2zE693;
      {    z2zE693 = (UINT64_C(1073741824) - za);
      }
      {    z3zE962 = (!(z2zE693 < zb));
      }
    } else {  z3zE962 = false;  }
    z2zE695 = z3zE962;
  }
  if (z2zE695) {
    {
      sail_u128 z3zE3818;
      z3zE3818 = u128_of_u64(za);
      sail_u128 z3zE3819;
      z3zE3819 = u128_add_u64(z3zE3818, zb);
      z8zE731 = u128_to_u64(z3zE3819);
    }
  } else {
    struct zexception z2zE696;
    CREATE(zexception)(&z2zE696);
    zInvalidBlock(&z2zE696, zBlockAccessListTooLarge);
    COPY(zexception)(current_exception, z2zE696);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/block_access_list.sail:108.8-108.51");
    KILL(zexception)(&z2zE696);
    goto end_block_exception_1464;
    /* unreachable after throw */
    KILL(zexception)(&z2zE696);
  }
end_function_1463: ;
  return z8zE731;
end_block_exception_1464: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zbal_count_addzIreprzGU64zCU64zCU64zKzIboundszGd0328719294598864996af0723f2d8e9zK(uint64_t za, uint64_t zb)
{
  uint64_t z8zE732;
  bool z2zE695;
  {
    bool z2zE694;
    {    z2zE694 = (!(UINT64_C(1073741824) < za));
    }
    bool z3zE962;
    if (z2zE694) {
      __int128 z2zE693;
      {
        __int128 z3zE3815;
        z3zE3815 = (__int128)(za);
        z2zE693 = (((__int128)((((unsigned __int128)UINT64_C(0)) << 64) | UINT64_C(1073741824))) - z3zE3815);
      }
      z3zE962 = (!(z2zE693 < zb));
    } else {  z3zE962 = false;  }
    z2zE695 = z3zE962;
  }
  if (z2zE695) {
    {
      sail_u128 z3zE3816;
      z3zE3816 = u128_of_u64(za);
      sail_u128 z3zE3817;
      z3zE3817 = u128_add_u64(z3zE3816, zb);
      z8zE732 = u128_to_u64(z3zE3817);
    }
  } else {
    struct zexception z2zE696;
    CREATE(zexception)(&z2zE696);
    zInvalidBlock(&z2zE696, zBlockAccessListTooLarge);
    COPY(zexception)(current_exception, z2zE696);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/block_access_list.sail:108.8-108.51");
    KILL(zexception)(&z2zE696);
    goto end_block_exception_1464;
    /* unreachable after throw */
    KILL(zexception)(&z2zE696);
  }
end_function_1463: ;
  return z8zE732;
end_block_exception_1464: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zbal_rlp_length_addzIreprzGU64zCR__sail_c_repr_u128zCU64zKzIboundszGebfa7455a07eda44e5082c02fe62be56zK(uint64_t zcurrent, sail_u128 zaddition)
{
  uint64_t z8zE733;
  uint64_t zbounded_addition;
  {
    zbounded_addition = zbal_rlp_length_from_byte_lengthzIreprzGR__sail_c_repr_u128zCU64zKzIboundszG6a2554a88fdc8146abe3845d0ca1cc6azK(zaddition);
    if (have_exception) {  goto end_block_exception_1472;  }
  }
  bool z2zE702;
  {
    uint64_t z2zE701;
    {    z2zE701 = (UINT64_C(1073741824) - zcurrent);
    }
    z2zE702 = (!(z2zE701 < zbounded_addition));
  }
  if (z2zE702) {
    {    z8zE733 = (zcurrent + zbounded_addition);
    }
  } else {
    struct zexception z2zE703;
    CREATE(zexception)(&z2zE703);
    zInvalidBlock(&z2zE703, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE703);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/block_access_list.sail:52.8-52.44");
    KILL(zexception)(&z2zE703);
    goto end_block_exception_1472;
    /* unreachable after throw */
    KILL(zexception)(&z2zE703);
  }
end_function_1471: ;
  return z8zE733;
end_block_exception_1472: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zbal_rlp_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(uint64_t zcurrent, uint64_t zaddition)
{
  uint64_t z8zE734;
  uint64_t zbounded_addition;
  {
    zbounded_addition = zbal_rlp_length_from_byte_lengthzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zaddition);
    if (have_exception) {  goto end_block_exception_1472;  }
  }
  bool z2zE702;
  {
    uint64_t z2zE701;
    {    z2zE701 = (UINT64_C(1073741824) - zcurrent);
    }
    z2zE702 = (!(z2zE701 < zbounded_addition));
  }
  if (z2zE702) {
    {    z8zE734 = (zcurrent + zbounded_addition);
    }
  } else {
    struct zexception z2zE703;
    CREATE(zexception)(&z2zE703);
    zInvalidBlock(&z2zE703, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE703);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/block_access_list.sail:52.8-52.44");
    KILL(zexception)(&z2zE703);
    goto end_block_exception_1472;
    /* unreachable after throw */
    KILL(zexception)(&z2zE703);
  }
end_function_1471: ;
  return z8zE734;
end_block_exception_1472: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zbal_rlp_length_from_byte_lengthzIreprzGR__sail_c_repr_u128zCU64zKzIboundszG6a2554a88fdc8146abe3845d0ca1cc6azK(sail_u128 zvalue)
{
  uint64_t z8zE735;
  bool z2zE704;
  {    z2zE704 = (!u128_lt(((sail_u128){{UINT64_C(1073741824), UINT64_C(0)}}), zvalue));
  }
  if (z2zE704) {  z8zE735 = u128_to_u64(zvalue);  } else {
    struct zexception z2zE705;
    CREATE(zexception)(&z2zE705);
    zInvalidBlock(&z2zE705, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE705);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/block_access_list.sail:35.8-35.44");
    KILL(zexception)(&z2zE705);
    goto end_block_exception_1476;
    /* unreachable after throw */
    KILL(zexception)(&z2zE705);
  }
end_function_1475: ;
  return z8zE735;
end_block_exception_1476: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zbal_rlp_length_from_byte_lengthzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zvalue)
{
  uint64_t z8zE736;
  bool z2zE704;
  {    z2zE704 = (!(UINT64_C(1073741824) < zvalue));
  }
  if (z2zE704) {  z8zE736 = zvalue;  } else {
    struct zexception z2zE705;
    CREATE(zexception)(&z2zE705);
    zInvalidBlock(&z2zE705, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE705);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/executor/block_access_list.sail:35.8-35.44");
    KILL(zexception)(&z2zE705);
    goto end_block_exception_1476;
    /* unreachable after throw */
    KILL(zexception)(&z2zE705);
  }
end_function_1475: ;
  return z8zE736;
end_block_exception_1476: ;

  return UINT64_C(0xdeadc0de);
}

unit zbal_write_accountszIreprzGU64zCU64zCuzKzIboundszG491a746de554142e7d65e0bb42a9e751zK(uint64_t zaccount_count, uint64_t zaccount)
{
  unit z8zE737;
  bool z2zE527;
  {    z2zE527 = (zaccount < zaccount_count);
  }
  if (z2zE527) {
    unit z3zE914;
    {
      z3zE914 = zbal_write_account(zaccount);
      if (have_exception) {  goto end_block_exception_1380;  }
    }
    uint64_t z2zE528;
    {
      sail_u128 z3zE3822;
      z3zE3822 = u128_of_u64(zaccount);
      sail_u128 z3zE3823;
      z3zE3823 = u128_add_u64(z3zE3822, UINT64_C(1));
      z2zE528 = u128_to_u64(z3zE3823);
    }
    {
      z8zE737 = zbal_write_accountszIreprzGU64zCU64zCuzKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zaccount_count, z2zE528);
      if (have_exception) {  goto end_block_exception_1380;  }
    }
  } else {  z8zE737 = UNIT;  }
end_function_1379: ;
  return z8zE737;
end_block_exception_1380: ;

  return UNIT;
}

unit zbal_write_accountszIreprzGU64zCU64zCuzKzIboundszG6940b3315867b733b4ae91c7ea9466eczK(uint64_t zaccount_count, uint64_t zaccount)
{
  unit z8zE738;
  bool z2zE527;
  {    z2zE527 = (zaccount < zaccount_count);
  }
  if (z2zE527) {
    unit z3zE914;
    {
      z3zE914 = zbal_write_account(zaccount);
      if (have_exception) {  goto end_block_exception_1380;  }
    }
    uint64_t z2zE528;
    {    z2zE528 = (UINT64_C(1) + zaccount);
    }
    {
      z8zE738 = zbal_write_accountszIreprzGU64zCU64zCuzKzIboundszG6940b3315867b733b4ae91c7ea9466eczK(zaccount_count, z2zE528);
      if (have_exception) {  goto end_block_exception_1380;  }
    }
  } else {  z8zE738 = UNIT;  }
end_function_1379: ;
  return z8zE738;
end_block_exception_1380: ;

  return UNIT;
}

sail_u256 zblob_word_mulzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256 zleft, uint64_t zright)
{
  sail_u256 z8zE739;
  bool z2zE2826;
  z2zE2826 = zword_is_zzero(zleft);
  if (z2zE2826) {  z8zE739 = zWORD_ZERO;  } else {
    bool z2zE2828;
    {
      sail_u256 z2zE2827;
      z2zE2827 = zword_div_word(zWORD_ALL_ONES, zleft);
      z2zE2828 = zword_ulezIreprzGU64zCR__sail_c_repr_u256zCozKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zright, z2zE2827);
    }
    if (z2zE2828) {
      z8zE739 = zword_mul_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zleft, zright);
    } else {
      struct zexception z2zE2829;
      CREATE(zexception)(&z2zE2829);
      zInvalidBlock(&z2zE2829, zExecutionInvalid);
      COPY(zexception)(current_exception, z2zE2829);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/evm/gas.sail:94.12-94.48");
      KILL(zexception)(&z2zE2829);
      goto end_block_exception_2915;
      /* unreachable after throw */
      KILL(zexception)(&z2zE2829);
    }
  }
end_function_2914: ;
  return z8zE739;
end_block_exception_2915: ;

  return u256_zero();
}

uint64_t zcapped_transaction_refundzIreprzGI128zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(__int128 ztotal, uint64_t zcap)
{
  uint64_t z8zE740;
  bool z2zE1651;
  z2zE1651 = (!(UINT64_C(0) < ztotal));
  if (z2zE1651) {  z8zE740 = UINT64_C(0);  } else {
    bool z2zE1652;
    z2zE1652 = (!(UINT64_C(18446744073709551615) < ztotal));
    if (z2zE1652) {
      uint64_t zadmitted_total;
      zadmitted_total = (uint64_t)(ztotal);
      bool z2zE1653;
      {
        __int128 z3zE3811;
        z3zE3811 = (__int128)(zadmitted_total);
        __int128 z3zE3812;
        z3zE3812 = (__int128)(zcap);
        z2zE1653 = (!(z3zE3812 < z3zE3811));
      }
      if (z2zE1653) {  z8zE740 = zadmitted_total;  } else {  z8zE740 = zcap;  }
    } else {  z8zE740 = zcap;  }
  }
end_function_2026: ;
  return z8zE740;
end_block_exception_2027: ;

  return UINT64_C(0xdeadc0de);
}

unit zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zamount)
{
  unit z8zE741;
  bool z2zE2780;
  {
    bool z2zE2779;
    {
      bool z2zE2778;
      z2zE2778 = zis_running(UNIT);
      z2zE2779 = not(z2zE2778);
    }
    bool z3zE2906;
    if (z2zE2779) {  z3zE2906 = true;  } else {
      {    z3zE2906 = (zamount == UINT64_C(0));
      }
    }
    z2zE2780 = z3zE2906;
  }
  if (z2zE2780) {  z8zE741 = UNIT;  } else {
    uint64_t zremaining;
    zremaining = zgas_remaining;
    bool z2zE2781;
    {    z2zE2781 = (!(zremaining < zamount));
    }
    if (z2zE2781) {
      {
        __int128 z3zE3805;
        z3zE3805 = (__int128)(zremaining);
        __int128 z3zE3806;
        z3zE3806 = (__int128)(zamount);
        __int128 z3zE3807;
        z3zE3807 = (z3zE3805 - z3zE3806);
        zgas_remaining = (uint64_t)(z3zE3807);
      }
      z8zE741 = UNIT;
    } else {
      {
        z8zE741 = zexc_halt(zOutOfGas);
        if (have_exception) {  goto end_block_exception_2879;  }
      }
    }
  }
end_function_2878: ;
  return z8zE741;
end_block_exception_2879: ;

  return UNIT;
}

unit zcharge_deployment_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zamount)
{
  unit z8zE742;
  bool z2zE2766;
  {
    bool z2zE2765;
    {
      z2zE2765 = zdebit_state_gaszIreprzGU64zCozKzIboundszGf087e944175a07f4d466ed4f31635cefzK(zamount);
      if (have_exception) {  goto end_block_exception_2865;  }
    }
    z2zE2766 = not(z2zE2765);
  }
  if (z2zE2766) {
    {
      z8zE742 = zexc_halt(zOutOfGas);
      if (have_exception) {  goto end_block_exception_2865;  }
    }
  } else {  z8zE742 = UNIT;  }
end_function_2864: ;
  return z8zE742;
end_block_exception_2865: ;

  return UNIT;
}

struct zMemoryRangeFields zcharge_memory_rangezIreprzGR__sail_c_repr_u256zCU64zCRMemoryRangeFieldszKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256 zstart, uint64_t zsizze)
{
  struct zMemoryRangeFields z8zE743;
  bool z2zE2727;
  z2zE2727 = zis_running(UNIT);
  if (z2zE2727) {
    uint64_t zavailable;
    zavailable = zgas_remaining;
    struct zMemoryExpansion zplan;
    {
      zplan = zmemory_expansionzIreprzGR__sail_c_repr_u256zCU64zCU64zCRMemoryExpansionzKzIboundszG14423a7d1e032b2eb688e7c175cd02a1zK(zstart, zsizze, zavailable);
      if (have_exception) {  goto end_block_exception_2825;  }
    }
    uint64_t z2zE2728;
    z2zE2728 = zplan.zcost;
    unit z3zE2858;
    {
      z3zE2858 = zchargezIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(z2zE2728);
      if (have_exception) {  goto end_block_exception_2825;  }
    }
    bool z2zE2729;
    z2zE2729 = zis_running(UNIT);
    if (z2zE2729) {  z8zE743 = zapply_memory_expansion(zplan);  } else {  z8zE743 = zEMPTY_MEMORY_RANGE;  }
  } else {  z8zE743 = zEMPTY_MEMORY_RANGE;  }
end_function_2824: ;
  return z8zE743;
end_block_exception_2825: ;
  struct zMemoryRangeFields z8zE1154 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de) };
  return z8zE1154;
}

unit zcharge_state_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zamount)
{
  unit z8zE744;
  bool z2zE2769;
  {
    bool z2zE2768;
    z2zE2768 = zis_running(UNIT);
    bool z3zE2899;
    if (z2zE2768) {
      bool z2zE2767;
      {
        z2zE2767 = zdebit_state_gaszIreprzGU64zCozKzIboundszGf087e944175a07f4d466ed4f31635cefzK(zamount);
        if (have_exception) {  goto end_block_exception_2867;  }
      }
      z3zE2899 = not(z2zE2767);
    } else {  z3zE2899 = false;  }
    z2zE2769 = z3zE2899;
  }
  if (z2zE2769) {
    {
      z8zE744 = zexc_halt(zOutOfGas);
      if (have_exception) {  goto end_block_exception_2867;  }
    }
  } else {  z8zE744 = UNIT;  }
end_function_2866: ;
  return z8zE744;
end_block_exception_2867: ;

  return UNIT;
}

unit zcheck_execution_gaszIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zamount)
{
  unit z8zE745;
  uint64_t zremaining;
  zremaining = zgas_remaining;
  bool z2zE2777;
  {
    bool z2zE2776;
    z2zE2776 = zis_running(UNIT);
    bool z3zE2905;
    if (z2zE2776) {
      {    z3zE2905 = (zremaining < zamount);
      }
    } else {  z3zE2905 = false;  }
    z2zE2777 = z3zE2905;
  }
  if (z2zE2777) {
    {
      z8zE745 = zexc_halt(zOutOfGas);
      if (have_exception) {  goto end_block_exception_2877;  }
    }
  } else {  z8zE745 = UNIT;  }
end_function_2876: ;
  return z8zE745;
end_block_exception_2877: ;

  return UNIT;
}

bool zdebit_state_gaszIreprzGU64zCozKzIboundszGf087e944175a07f4d466ed4f31635cefzK(uint64_t zamount)
{
  bool z8zE746;
  bool z2zE2770;
  {    z2zE2770 = (zamount == UINT64_C(0));
  }
  unit z3zE2900;
  if (z2zE2770) {
    z8zE746 = true;
    goto cleanup_2869;
    /* unreachable after return */
    goto end_cleanup_2870;
  cleanup_2869: ;
    goto end_function_2868;
  end_cleanup_2870: ;
  } else {  z3zE2900 = UNIT;  }
  uint64_t zstate_left;
  zstate_left = zstate_gas_remaining;
  uint64_t zexecution_left;
  zexecution_left = zgas_remaining;
  bool z2zE2771;
  {    z2zE2771 = (!(zstate_left < zamount));
  }
  if (z2zE2771) {
    {
      __int128 z3zE3828;
      z3zE3828 = (__int128)(zstate_left);
      __int128 z3zE3829;
      z3zE3829 = (__int128)(zamount);
      __int128 z3zE3830;
      z3zE3830 = (z3zE3828 - z3zE3829);
      zstate_gas_remaining = (uint64_t)(z3zE3830);
    }
    unit z3zE2904;
    z3zE2904 = UNIT;
    z8zE746 = true;
  } else {
    __int128 zremainder;
    {
      __int128 z3zE3831;
      z3zE3831 = (__int128)(zamount);
      __int128 z3zE3832;
      z3zE3832 = (__int128)(zstate_left);
      zremainder = (z3zE3831 - z3zE3832);
    }
    bool z2zE2772;
    z2zE2772 = (!(zexecution_left < zremainder));
    if (z2zE2772) {
      uint64_t zspilled;
      zspilled = zstate_gas_spilled;
      zstate_gas_remaining = zGAS_ZERO;
      unit z3zE2903;
      z3zE2903 = UNIT;
      {
        __int128 z3zE3833;
        z3zE3833 = (__int128)(zexecution_left);
        __int128 z3zE3834;
        z3zE3834 = (z3zE3833 - zremainder);
        zgas_remaining = (uint64_t)(z3zE3834);
      }
      unit z3zE2902;
      z3zE2902 = UNIT;
      {
        zstate_gas_spilled = zstate_gas_spill_addzIreprzGU64zCI128zCU64zKzIboundszG1c0bd00f9b9bd1c00190715feb145f1dzK(zspilled, zremainder);
        if (have_exception) {  goto end_block_exception_2871;  }
      }
      unit z3zE2901;
      z3zE2901 = UNIT;
      z8zE746 = true;
    } else {  z8zE746 = false;  }
  }
end_function_2868: ;
  return z8zE746;
end_block_exception_2871: ;

  return false;
}

bool zdeployed_code_sizze_allowedzIreprzGU64zCozKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zsizze)
{
  bool z8zE747;
  bool z2zE2785;
  z2zE2785 = zfork_lt(zk_fork, zByzzantium);
  if (z2zE2785) {  z8zE747 = true;  } else {
    bool z2zE2786;
    z2zE2786 = zfork_lt(zk_fork, zAmsterdam);
    if (z2zE2786) {
      {    z8zE747 = (!(zEIP170_DEPLOYED_CODE_SIZE_LIMIT < zsizze));
      }
    } else {
      {    z8zE747 = (!(zEIP7954_DEPLOYED_CODE_SIZE_LIMIT < zsizze));
      }
    }
  }
end_function_2884: ;
  return z8zE747;
end_block_exception_2885: ;

  return false;
}

void zfixed_precompile_gaszIreprzGU64zCU64zCOzIU64zKzKzIboundszG1d764f9bc0058f5fbf2b37ea4b7189f7zK(struct zoptionzIU64zK *z8zE748, uint64_t zcost, uint64_t zavailable)
{
  bool z2zE2657;
  {    z2zE2657 = (!(zavailable < zcost));
  }
  if (z2zE2657) {
    uint64_t zaffordable;
    zaffordable = zcost;
    zSomezIU64zK((*(&z8zE748)), zaffordable);
  } else {  zNonezIU64zK((*(&z8zE748)), UNIT);  }
end_function_2762: ;
  goto end_function_4011;
end_block_exception_2763: ;
  goto end_function_4011;
end_function_4011: ;
}

sail_u256 zfrom_bytes_lezIreprzGU64zCR__sail_c_repr_fixed_byteszIC20zKzCR__sail_c_repr_u256zKzIboundszG83613fe416f377787be94d88c23f4a78zK(uint64_t zn, sail_fixed_bytes_20 zv)
{
  sail_u256 z8zE749;
  z8zE749 = u256_from_fixed_bytes_20(zv);
  return z8zE749;
}

sail_u256 zfrom_bytes_lezIreprzGU64zCR__sail_c_repr_fixed_byteszIC32zKzCR__sail_c_repr_u256zKzIboundszG78c09f65c90525db636e344fe928bc0bzK(uint64_t zn, sail_fixed_bytes_32 zv)
{
  sail_u256 z8zE750;
  z8zE750 = u256_from_fixed_bytes_32(zv);
  return z8zE750;
}

bool zinitcode_sizze_allowedzIreprzGU64zCozKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zsizze)
{
  bool z8zE751;
  bool z2zE2783;
  z2zE2783 = zfork_lt(zk_fork, zShanghai);
  if (z2zE2783) {  z8zE751 = true;  } else {
    bool z2zE2784;
    z2zE2784 = zfork_lt(zk_fork, zAmsterdam);
    if (z2zE2784) {
      {    z8zE751 = (!(zEIP3860_INITCODE_SIZE_LIMIT < zsizze));
      }
    } else {
      {    z8zE751 = (!(zEIP7954_INITCODE_SIZE_LIMIT < zsizze));
      }
    }
  }
end_function_2882: ;
  return z8zE751;
end_block_exception_2883: ;

  return false;
}

unit zk_add_balancezIreprzGR__sail_c_repr_fixed_byteszIC20zKzCR__sail_c_repr_u128zCuzKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_fixed_bytes_20 za, sail_u128 zv)
{
  unit z8zE752;
  struct zAccount zcur;
  {
    zcur = zk_aload(za);
    if (have_exception) {  goto end_block_exception_3075;  }
  }
  bool z2zE2983;
  {
    bool z2zE2982;
    z2zE2982 = zword_is_zzerozIreprzGR__sail_c_repr_u128zCozKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zv);
    z2zE2983 = not(z2zE2982);
  }
  if (z2zE2983) {
    struct zAccountInfo z2zE2988;
    {
      struct zAccountInfo z2zE2984;
      z2zE2984 = zcur.zinfo;
      sail_u256 z2zE2987;
      {
        sail_u256 z2zE2986;
        {
          struct zAccountInfo z2zE2985;
          z2zE2985 = zcur.zinfo;
          z2zE2986 = z2zE2985.zbalance;
        }
        z2zE2987 = zalu_addzIreprzGR__sail_c_repr_u256zCR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE2986, zv);
      }
      struct zAccountInfo z3zE3035;
      z3zE3035 = z2zE2984;
      z3zE3035.zbalance = z2zE2987;
      z2zE2988 = z3zE3035;
    }
    z8zE752 = zstore_account_info(za, zcur, z2zE2988);
  } else {  z8zE752 = UNIT;  }
end_function_3074: ;
  return z8zE752;
end_block_exception_3075: ;

  return UNIT;
}

unit zk_add_balancezIreprzGR__sail_c_repr_fixed_byteszIC20zKzCU64zCuzKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_fixed_bytes_20 za, uint64_t zv)
{
  unit z8zE753;
  struct zAccount zcur;
  {
    zcur = zk_aload(za);
    if (have_exception) {  goto end_block_exception_3075;  }
  }
  bool z2zE2983;
  {
    bool z2zE2982;
    z2zE2982 = zword_is_zzerozIreprzGU64zCozKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zv);
    z2zE2983 = not(z2zE2982);
  }
  if (z2zE2983) {
    struct zAccountInfo z2zE2988;
    {
      struct zAccountInfo z2zE2984;
      z2zE2984 = zcur.zinfo;
      sail_u256 z2zE2987;
      {
        sail_u256 z2zE2986;
        {
          struct zAccountInfo z2zE2985;
          z2zE2985 = zcur.zinfo;
          z2zE2986 = z2zE2985.zbalance;
        }
        z2zE2987 = zalu_addzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(z2zE2986, zv);
      }
      struct zAccountInfo z3zE3035;
      z3zE3035 = z2zE2984;
      z3zE3035.zbalance = z2zE2987;
      z2zE2988 = z3zE3035;
    }
    z8zE753 = zstore_account_info(za, zcur, z2zE2988);
  } else {  z8zE753 = UNIT;  }
end_function_3074: ;
  return z8zE753;
end_block_exception_3075: ;

  return UNIT;
}

struct zMemoryExpansion zmemory_expansionzIreprzGR__sail_c_repr_u256zCU64zCU64zCRMemoryExpansionzKzIboundszG14423a7d1e032b2eb688e7c175cd02a1zK(sail_u256 zstart, uint64_t zsizze, uint64_t zavailable)
{
  struct zMemoryExpansion z8zE754;
  bool z2zE2743;
  z2zE2743 = (zsizze == UINT64_C(0));
  if (z2zE2743) {
    struct zMemoryExpansion z3zE2890;
    z3zE2890.zcost = UINT64_C(0);
    z3zE2890.zrange = zEMPTY_MEMORY_RANGE;
    z3zE2890.zrequired_sizze = UINT64_C(0);
    z8zE754 = z3zE2890;
  } else {
    bool z2zE2745;
    {
      bool z2zE2744;
      z2zE2744 = (!u64_lt_u256(UINT64_C(18446744073709551615), zstart));
      bool z3zE2876;
      if (z2zE2744) {  z3zE2876 = (!(UINT64_C(18446744073709551615) < zsizze));  } else {  z3zE2876 = false;  }
      z2zE2745 = z3zE2876;
    }
    if (z2zE2745) {
      struct zMemoryExpansion z3zE2879;
      {
        uint64_t zbounded_start;
        zbounded_start = u256_to_u64(zstart);
        struct zMemoryExpansion z3zE2880;
        {
          uint64_t zbounded_sizze;
          zbounded_sizze = zsizze;
          sail_u128 zrequired;
          zrequired = u128_add_u64_u64(zbounded_start, zbounded_sizze);
          uint64_t z2zE2746;
          z2zE2746 = zmemory_word_countzIreprzGR__sail_c_repr_u128zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zrequired);
          struct zMemoryExpansion z3zE2881;
          {
            uint64_t zwords;
            zwords = z2zE2746;
            struct zMemoryExpansion z3zE2882;
            {
              uint64_t zbounded_words;
              zbounded_words = zwords;
              sail_u128 zexpansion_cost;
              zexpansion_cost = zbounded_mem_cost(zbounded_words);
              bool z2zE2747;
              z2zE2747 = (!u64_lt_u128(UINT64_C(18446744073709551615), zexpansion_cost));
              if (z2zE2747) {
                uint64_t zrequired_sizze;
                zrequired_sizze = u128_to_u64(zrequired);
                struct zMemoryRangeFields z2zE2748;
                z2zE2748 = zmemory_range(zbounded_start, zbounded_sizze);
                uint64_t z2zE2749;
                {
                  z2zE2749 = zmemory_expansion_cost(zrequired_sizze, zavailable);
                  if (have_exception) {  goto end_block_exception_2847;  }
                }
                struct zMemoryExpansion z3zE2886;
                z3zE2886.zcost = z2zE2749;
                z3zE2886.zrange = z2zE2748;
                z3zE2886.zrequired_sizze = zrequired_sizze;
                z3zE2882 = z3zE2886;
              } else {
                unit z3zE2884;
                {
                  z3zE2884 = zexc_halt(zOutOfGas);
                  if (have_exception) {  goto end_block_exception_2847;  }
                }
                struct zMemoryExpansion z3zE2885;
                z3zE2885.zcost = UINT64_C(0);
                z3zE2885.zrange = zEMPTY_MEMORY_RANGE;
                z3zE2885.zrequired_sizze = UINT64_C(0);
                z3zE2882 = z3zE2885;
              }
              goto finish_match_2844;
            }
          case_2845: ;
            sail_match_failure("memory_expansion<repr:R__sail_c_repr_u256,U64,U64,RMemoryExpansion><bounds:14423a7d1e032b2eb688e7c175cd02a1>");
          finish_match_2844: ;
            z3zE2881 = z3zE2882;
            goto finish_match_2842;
          }
        case_2843: ;
          sail_match_failure("memory_expansion<repr:R__sail_c_repr_u256,U64,U64,RMemoryExpansion><bounds:14423a7d1e032b2eb688e7c175cd02a1>");
        finish_match_2842: ;
          z3zE2880 = z3zE2881;
          goto finish_match_2840;
        }
      case_2841: ;
        sail_match_failure("memory_expansion<repr:R__sail_c_repr_u256,U64,U64,RMemoryExpansion><bounds:14423a7d1e032b2eb688e7c175cd02a1>");
      finish_match_2840: ;
        z3zE2879 = z3zE2880;
        goto finish_match_2838;
      }
    case_2839: ;
      sail_match_failure("memory_expansion<repr:R__sail_c_repr_u256,U64,U64,RMemoryExpansion><bounds:14423a7d1e032b2eb688e7c175cd02a1>");
    finish_match_2838: ;
      z8zE754 = z3zE2879;
    } else {
      unit z3zE2877;
      {
        z3zE2877 = zexc_halt(zOutOfGas);
        if (have_exception) {  goto end_block_exception_2847;  }
      }
      struct zMemoryExpansion z3zE2878;
      z3zE2878.zcost = UINT64_C(0);
      z3zE2878.zrange = zEMPTY_MEMORY_RANGE;
      z3zE2878.zrequired_sizze = UINT64_C(0);
      z8zE754 = z3zE2878;
    }
  }
end_function_2846: ;
  return z8zE754;
end_block_exception_2847: ;
  struct zMemoryRangeFields z8zE1156 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de) };
  struct zMemoryExpansion z8zE1155 = { .zcost = UINT64_C(0xdeadc0de), .zrange = z8zE1156, .zrequired_sizze = UINT64_C(0xdeadc0de) };
  return z8zE1155;
}

uint64_t zmemory_word_countzIreprzGR__sail_c_repr_u128zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(sail_u128 zbyte_len)
{
  uint64_t z8zE755;
  sail_u256 z2zE2760;
  {
    sail_u256 z3zE3803;
    z3zE3803 = u256_of_u128(zbyte_len);
    z2zE2760 = u256_add_u64(z3zE3803, UINT64_C(31));
  }
  {
    sail_u256 z3zE3804;
    z3zE3804 = u256_div_u64(z2zE2760, UINT64_C(32));
    z8zE755 = u256_to_u64(z3zE3804);
  }
end_function_2854: ;
  return z8zE755;
end_block_exception_2855: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zmemory_word_countzIreprzGU64zCU64zKzIboundszG07d9444e226eec3cde1a1e781c91ddf4zK(uint64_t zbyte_len)
{
  uint64_t z8zE756;
  uint64_t z2zE2760;
  {    z2zE2760 = (UINT64_C(31) + zbyte_len);
  }
  {    z8zE756 = (z2zE2760 / UINT64_C(32));
  }
end_function_2854: ;
  return z8zE756;
end_block_exception_2855: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zmemory_word_countzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zbyte_len)
{
  uint64_t z8zE757;
  sail_u128 z2zE2760;
  {
    sail_u128 z3zE3801;
    z3zE3801 = u128_of_u64(zbyte_len);
    z2zE2760 = u128_add_u64(z3zE3801, UINT64_C(31));
  }
  {
    sail_u128 z3zE3802;
    z3zE3802 = u128_div_u64(z2zE2760, UINT64_C(32));
    z8zE757 = u128_to_u64(z3zE3802);
  }
end_function_2854: ;
  return z8zE757;
end_block_exception_2855: ;

  return UINT64_C(0xdeadc0de);
}

void zminimal_word_byteszIreprzGU64zCz8LB8zCU64z9zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 *z8zE758, uint64_t zw)
{
  sail_u256 zremaining;
  zremaining = u256_of_fbits(zw);
  zz5listz8z5bv8z9 zout;
  CREATE(zz5listz8z5bv8z9)(&zout);
  {
  }
  uint64_t zlen;
  zlen = UINT64_C(0);
  int64_t z3zE3624;
  {    z3zE3624 = (int64_t)(UINT64_C(0));
  }
  int64_t z3zE3625;
  {    z3zE3625 = (int64_t)(UINT64_C(31));
  }
  int64_t z3zE3626;
  {    z3zE3626 = (int64_t)(UINT64_C(1));
  }
  {
    int64_t zbyte_index;
    zbyte_index = z3zE3624;
    unit z3zE3627;
  for_start_3634: ;
    {
      if ((z3zE3625 < zbyte_index)) goto for_end_3635;
      bool z2zE3780;
      z2zE3780 = zword_nonzzero(zremaining);
      if (z2zE3780) {
        uint64_t z2zE3781;
        z2zE3781 = zword_low_byte(zremaining);
        zconsz3z5bv8(&zout, z2zE3781, zout);
        unit z3zE3629;
        z3zE3629 = UNIT;
        zremaining = zword_shift_right(zremaining, UINT64_C(8));
        unit z3zE3628;
        z3zE3628 = UNIT;
        {
          uint64_t z3zE3871;
          z3zE3871 = (uint64_t)(zbyte_index);
          zlen = zword_byte_count(z3zE3871);
        }
        z3zE3627 = UNIT;
      } else {  z3zE3627 = UNIT;  }
      zbyte_index = (zbyte_index + z3zE3626);
      goto for_start_3634;
    }
  for_end_3635: ;
  }
  unit z3zE3630;
  z3zE3630 = UNIT;
  struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z3zE3631;
  CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE3631);
  COPY(zz5listz8z5bv8z9)(&((&z3zE3631)->ztup0), zout);
  z3zE3631.ztup1 = zlen;
  COPY(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)((*(&z8zE758)), z3zE3631);
  KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE3631);
  KILL(zz5listz8z5bv8z9)(&zout);
end_function_3636: ;
  goto end_function_4010;
end_block_exception_3637: ;
  goto end_function_4010;
end_function_4010: ;
}

unit zrecord_refundzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zdelta)
{
  unit z8zE759;
  {
    zframe_refund = zvalidated_refund_addzIreprzGI128zCU64zCI128zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zframe_refund, zdelta);
    if (have_exception) {  goto end_block_exception_2998;  }
  }
  z8zE759 = UNIT;
end_function_2997: ;
  return z8zE759;
end_block_exception_2998: ;

  return UNIT;
}

uint64_t zrlp_bytes_sizzezIreprzGLB8zCU64zCU64zKzIboundszGed754718030b429454cf327fc49bd29dzK(zz5listz8z5bv8z9 zdata, uint64_t zlen)
{
  uint64_t z8zE760;
  uint64_t zfirst;
  {
    uint64_t z3zE3606;
    {
      if ((zdata == NULL)) goto case_3613;
      uint64_t zb;
      zb = (*zdata).hd;
      z3zE3606 = zb;
      goto finish_match_3611;
    }
  case_3613: ;
    {
      /* complete */
      z3zE3606 = UINT64_C(0x00);
      goto finish_match_3611;
    }
  case_3612: ;
  finish_match_3611: ;
    zfirst = z3zE3606;
  }
  z8zE760 = zrlp_string_sizzezIreprzGU64zCB8zCU64zKzIboundszG69b6fbdbe405b82ee0c43387d0dda91azK(zlen, zfirst);
end_function_3614: ;
  return z8zE760;
end_block_exception_3615: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_item_count_incrementzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zvalue)
{
  uint64_t z8zE761;
  uint64_t z3zE3475;
  z3zE3475 = (zvalue + ((__int128)((((unsigned __int128)UINT64_C(0)) << 64) | UINT64_C(1))));
  z8zE761 = z3zE3475;
end_function_3457: ;
  return z8zE761;
end_block_exception_3458: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_length_prefix_lenzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zlen)
{
  uint64_t z8zE762;
  bool z2zE3754;
  {    z2zE3754 = (!(zRLP_SHORT_LENGTH_LIMIT < zlen));
  }
  if (z2zE3754) {  z8zE762 = UINT64_C(1);  } else {
    uint64_t z2zE3756;
    {
      sail_u256 z2zE3755;
      z2zE3755 = zrlp_length_wordzIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zlen);
      z2zE3756 = zrlp_minimal_word_len(z2zE3755);
    }
    {    z8zE762 = (UINT64_C(1) + z2zE3756);
    }
  }
end_function_3618: ;
  return z8zE762;
end_block_exception_3619: ;

  return UINT64_C(0xdeadc0de);
}

sail_u256 zrlp_length_wordzIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zvalue)
{
  sail_u256 z8zE763;
  z8zE763 = zword_of_nat_byte_countzIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zvalue);
end_function_3630: ;
  return z8zE763;
end_block_exception_3631: ;

  return u256_zero();
}

void zrlp_list_prefixzIreprzGU64zCz8LB8zCU64z9zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 *z8zE764, uint64_t zlen)
{
  bool z2zE3757;
  {    z2zE3757 = (!(zRLP_SHORT_LENGTH_LIMIT < zlen));
  }
  if (z2zE3757) {
    zz5listz8z5bv8z9 z2zE3760;
    CREATE(zz5listz8z5bv8z9)(&z2zE3760);
    {
      uint64_t z2zE3759;
      {
        uint64_t z2zE3758;
        z2zE3758 = zrlp_nat_length_byte(zlen);
        z2zE3759 = ((UINT64_C(0xC0) + z2zE3758) & UINT64_C(0xFF));
      }
      zconsz3z5bv8(&z2zE3760, z2zE3759, z2zE3760);
    }
    struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z3zE3614;
    CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE3614);
    COPY(zz5listz8z5bv8z9)(&((&z3zE3614)->ztup0), z2zE3760);
    z3zE3614.ztup1 = UINT64_C(1);
    COPY(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)((*(&z8zE764)), z3zE3614);
    KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE3614);
    KILL(zz5listz8z5bv8z9)(&z2zE3760);
  } else {
    struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z2zE3762;
    CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE3762);
    {
      sail_u256 z2zE3761;
      z2zE3761 = zrlp_length_wordzIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zlen);
      zminimal_word_bytes(&z2zE3762, z2zE3761);
    }
    struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z3zE3610;
    CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE3610);
    {
      zz5listz8z5bv8z9 zlength_bytes;
      CREATE(zz5listz8z5bv8z9)(&zlength_bytes);
      COPY(zz5listz8z5bv8z9)(&zlength_bytes, z2zE3762.ztup0);
      uint64_t zlength_len;
      zlength_len = z2zE3762.ztup1;
      zz5listz8z5bv8z9 z2zE3765;
      CREATE(zz5listz8z5bv8z9)(&z2zE3765);
      {
        uint64_t z2zE3764;
        {
          uint64_t z2zE3763;
          z2zE3763 = zrlp_nat_length_byte(zlength_len);
          z2zE3764 = ((UINT64_C(0xF7) + z2zE3763) & UINT64_C(0xFF));
        }
        zconsz3z5bv8(&z2zE3765, z2zE3764, zlength_bytes);
      }
      uint64_t z2zE3766;
      {    z2zE3766 = (UINT64_C(1) + zlength_len);
      }
      COPY(zz5listz8z5bv8z9)(&((&z3zE3610)->ztup0), z2zE3765);
      z3zE3610.ztup1 = z2zE3766;
      KILL(zz5listz8z5bv8z9)(&z2zE3765);
      KILL(zz5listz8z5bv8z9)(&zlength_bytes);
      goto finish_match_3622;
    }
  case_3623: ;
    sail_match_failure("rlp_list_prefix<repr:U64,(LB8,U64)><bounds:4c3f6287b16f25a07ff498da45d6ed37>");
  finish_match_3622: ;
    COPY(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)((*(&z8zE764)), z3zE3610);
    KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE3610);
    KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE3762);
  }
end_function_3624: ;
  goto end_function_4009;
end_block_exception_3625: ;
  goto end_function_4009;
end_function_4009: ;
}

uint64_t zrlp_list_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zcontent_len)
{
  uint64_t z8zE765;
  uint64_t z2zE3743;
  z2zE3743 = zrlp_length_prefix_lenzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zcontent_len);
  {
    sail_u128 z3zE3813;
    z3zE3813 = u128_of_u64(z2zE3743);
    sail_u128 z3zE3814;
    z3zE3814 = u128_add_u64(z3zE3813, zcontent_len);
    z8zE765 = u128_to_u64(z3zE3814);
  }
end_function_3603: ;
  return z8zE765;
end_block_exception_3604: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_minimal_word_lenzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zw)
{
  uint64_t z8zE766;
  z8zE766 = zword_byte_lengthzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zw);
end_function_3620: ;
  return z8zE766;
end_block_exception_3621: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_scratch_length_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(uint64_t zleft, uint64_t zright)
{
  uint64_t z8zE767;
  bool z2zE3783;
  {
    uint64_t z2zE3782;
    {    z2zE3782 = (UINT64_C(18446744073709551615) - zleft);
    }
    {    z2zE3783 = (!(z2zE3782 < zright));
    }
  }
  if (z2zE3783) {
    {
      sail_u128 z3zE3797;
      z3zE3797 = u128_of_u64(zleft);
      sail_u128 z3zE3798;
      z3zE3798 = u128_add_u64(z3zE3797, zright);
      z8zE767 = u128_to_u64(z3zE3798);
    }
  } else {
    struct zexception z2zE3784;
    CREATE(zexception)(&z2zE3784);
    zInvalidBlock(&z2zE3784, zRlpDecode);
    COPY(zexception)(current_exception, z2zE3784);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/splices/c_optimized.sail:158.8-158.37");
    KILL(zexception)(&z2zE3784);
    goto end_block_exception_3641;
    /* unreachable after throw */
    KILL(zexception)(&z2zE3784);
  }
end_function_3640: ;
  return z8zE767;
end_block_exception_3641: ;

  return UINT64_C(0xdeadc0de);
}

void zrlp_string_prefixzIreprzGU64zCB8zCz8LB8zCU64z9zKzIboundszG7d1a1d23d11135c1c1bff78d34e8a73dzK(struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 *z8zE768, uint64_t zlen, uint64_t zfirst)
{
  bool z2zE3769;
  {
    bool z2zE3768;
    {    z2zE3768 = (zlen == UINT64_C(1));
    }
    bool z3zE3615;
    if (z2zE3768) {
      uint64_t z2zE3767;
      z2zE3767 = (UINT64_C(1) & (zfirst >> UINT64_C(7)));
      z3zE3615 = (z2zE3767 == UINT64_C(0b0));
    } else {  z3zE3615 = false;  }
    z2zE3769 = z3zE3615;
  }
  if (z2zE3769) {
    zz5listz8z5bv8z9 z3zE3621;
    CREATE(zz5listz8z5bv8z9)(&z3zE3621);
    struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z3zE3622;
    CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE3622);
    COPY(zz5listz8z5bv8z9)(&((&z3zE3622)->ztup0), z3zE3621);
    z3zE3622.ztup1 = UINT64_C(0);
    COPY(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)((*(&z8zE768)), z3zE3622);
    KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE3622);
    KILL(zz5listz8z5bv8z9)(&z3zE3621);
  } else {
    bool z2zE3770;
    {    z2zE3770 = (!(zRLP_SHORT_LENGTH_LIMIT < zlen));
    }
    if (z2zE3770) {
      zz5listz8z5bv8z9 z2zE3773;
      CREATE(zz5listz8z5bv8z9)(&z2zE3773);
      {
        uint64_t z2zE3772;
        {
          uint64_t z2zE3771;
          z2zE3771 = zrlp_nat_length_byte(zlen);
          z2zE3772 = ((UINT64_C(0x80) + z2zE3771) & UINT64_C(0xFF));
        }
        zconsz3z5bv8(&z2zE3773, z2zE3772, z2zE3773);
      }
      struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z3zE3620;
      CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE3620);
      COPY(zz5listz8z5bv8z9)(&((&z3zE3620)->ztup0), z2zE3773);
      z3zE3620.ztup1 = UINT64_C(1);
      COPY(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)((*(&z8zE768)), z3zE3620);
      KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE3620);
      KILL(zz5listz8z5bv8z9)(&z2zE3773);
    } else {
      struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z2zE3775;
      CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE3775);
      {
        sail_u256 z2zE3774;
        z2zE3774 = zrlp_length_wordzIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zlen);
        zminimal_word_bytes(&z2zE3775, z2zE3774);
      }
      struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z3zE3616;
      CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE3616);
      {
        zz5listz8z5bv8z9 zlength_bytes;
        CREATE(zz5listz8z5bv8z9)(&zlength_bytes);
        COPY(zz5listz8z5bv8z9)(&zlength_bytes, z2zE3775.ztup0);
        uint64_t zlength_len;
        zlength_len = z2zE3775.ztup1;
        zz5listz8z5bv8z9 z2zE3778;
        CREATE(zz5listz8z5bv8z9)(&z2zE3778);
        {
          uint64_t z2zE3777;
          {
            uint64_t z2zE3776;
            z2zE3776 = zrlp_nat_length_byte(zlength_len);
            z2zE3777 = ((UINT64_C(0xB7) + z2zE3776) & UINT64_C(0xFF));
          }
          zconsz3z5bv8(&z2zE3778, z2zE3777, zlength_bytes);
        }
        uint64_t z2zE3779;
        {    z2zE3779 = (UINT64_C(1) + zlength_len);
        }
        COPY(zz5listz8z5bv8z9)(&((&z3zE3616)->ztup0), z2zE3778);
        z3zE3616.ztup1 = z2zE3779;
        KILL(zz5listz8z5bv8z9)(&z2zE3778);
        KILL(zz5listz8z5bv8z9)(&zlength_bytes);
        goto finish_match_3626;
      }
    case_3627: ;
      sail_match_failure("rlp_string_prefix<repr:U64,B8,(LB8,U64)><bounds:7d1a1d23d11135c1c1bff78d34e8a73d>");
    finish_match_3626: ;
      COPY(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)((*(&z8zE768)), z3zE3616);
      KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z3zE3616);
      KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE3775);
    }
  }
end_function_3628: ;
  goto end_function_4008;
end_block_exception_3629: ;
  goto end_function_4008;
end_function_4008: ;
}

sail_u128 zrlp_string_sizzezIreprzGU64zCB8zCR__sail_c_repr_u128zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(uint64_t zlen, uint64_t zfirst)
{
  sail_u128 z8zE769;
  bool z2zE3752;
  {
    bool z2zE3751;
    {    z2zE3751 = (zlen == UINT64_C(1));
    }
    bool z3zE3609;
    if (z2zE3751) {
      uint64_t z2zE3750;
      z2zE3750 = (UINT64_C(1) & (zfirst >> UINT64_C(7)));
      z3zE3609 = (z2zE3750 == UINT64_C(0b0));
    } else {  z3zE3609 = false;  }
    z2zE3752 = z3zE3609;
  }
  if (z2zE3752) {  z8zE769 = u128_of_u64(UINT64_C(1));  } else {
    uint64_t z2zE3753;
    z2zE3753 = zrlp_length_prefix_lenzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zlen);
    {
      sail_u128 z3zE3796;
      z3zE3796 = u128_of_u64(z2zE3753);
      z8zE769 = u128_add_u64(z3zE3796, zlen);
    }
  }
end_function_3616: ;
  return z8zE769;
end_block_exception_3617: ;

  return u128_zero();
}

uint64_t zrlp_string_sizzezIreprzGU64zCB8zCU64zKzIboundszG69b6fbdbe405b82ee0c43387d0dda91azK(uint64_t zlen, uint64_t zfirst)
{
  uint64_t z8zE770;
  bool z2zE3752;
  {
    bool z2zE3751;
    {    z2zE3751 = (zlen == UINT64_C(1));
    }
    bool z3zE3609;
    if (z2zE3751) {
      uint64_t z2zE3750;
      z2zE3750 = (UINT64_C(1) & (zfirst >> UINT64_C(7)));
      z3zE3609 = (z2zE3750 == UINT64_C(0b0));
    } else {  z3zE3609 = false;  }
    z2zE3752 = z3zE3609;
  }
  if (z2zE3752) {  z8zE770 = UINT64_C(1);  } else {
    uint64_t z2zE3753;
    z2zE3753 = zrlp_length_prefix_lenzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zlen);
    {    z8zE770 = (zlen + z2zE3753);
    }
  }
end_function_3616: ;
  return z8zE770;
end_block_exception_3617: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zrlp_uint_word_sizzezIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zw)
{
  uint64_t z8zE771;
  uint64_t zlen;
  zlen = zrlp_minimal_word_lenzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zw);
  uint64_t zfirst;
  {
    bool z2zE3748;
    z2zE3748 = (zlen == UINT64_C(1));
    if (z2zE3748) {  zfirst = zword_low_bytezIreprzGU64zCB8zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zw);  } else {
      zfirst = UINT64_C(0x00);
    }
  }
  bool z2zE3747;
  {
    bool z2zE3746;
    z2zE3746 = (zlen == UINT64_C(1));
    bool z3zE3605;
    if (z2zE3746) {
      uint64_t z2zE3745;
      z2zE3745 = (UINT64_C(1) & (zfirst >> UINT64_C(7)));
      z3zE3605 = (z2zE3745 == UINT64_C(0b0));
    } else {  z3zE3605 = false;  }
    z2zE3747 = z3zE3605;
  }
  if (z2zE3747) {  z8zE771 = UINT64_C(1);  } else {
    {    z8zE771 = (UINT64_C(1) + zlen);
    }
  }
end_function_3607: ;
  return z8zE771;
end_block_exception_3608: ;

  return UINT64_C(0xdeadc0de);
}

unit zrlp_write_list_prefixzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zcontent_len)
{
  unit z8zE772;
  struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z2zE3728;
  CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE3728);
  zrlp_list_prefixzIreprzGU64zCz8LB8zCU64z9zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(&z2zE3728, zcontent_len);
  unit z3zE3596;
  {
    zz5listz8z5bv8z9 zencoded_prefix;
    CREATE(zz5listz8z5bv8z9)(&zencoded_prefix);
    COPY(zz5listz8z5bv8z9)(&zencoded_prefix, z2zE3728.ztup0);
    uint64_t zprefix_len;
    zprefix_len = z2zE3728.ztup1;
    z3zE3596 = zscratch_push_bytes(zencoded_prefix, zprefix_len);
    KILL(zz5listz8z5bv8z9)(&zencoded_prefix);
    goto finish_match_3586;
  }
case_3587: ;
  sail_match_failure("rlp_write_list_prefix<repr:U64,u><bounds:4c3f6287b16f25a07ff498da45d6ed37>");
finish_match_3586: ;
  z8zE772 = z3zE3596;
  KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE3728);
end_function_3588: ;
  return z8zE772;
end_block_exception_3589: ;

  return UNIT;
}

unit zrlp_write_string_prefixzIreprzGU64zCB8zCuzKzIboundszG491a746de554142e7d65e0bb42a9e751zK(uint64_t zlen, uint64_t zfirst)
{
  unit z8zE773;
  struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z2zE3729;
  CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE3729);
  zrlp_string_prefixzIreprzGU64zCB8zCz8LB8zCU64z9zKzIboundszG7d1a1d23d11135c1c1bff78d34e8a73dzK(&z2zE3729, zlen, zfirst);
  unit z3zE3598;
  {
    zz5listz8z5bv8z9 zencoded_prefix;
    CREATE(zz5listz8z5bv8z9)(&zencoded_prefix);
    COPY(zz5listz8z5bv8z9)(&zencoded_prefix, z2zE3729.ztup0);
    uint64_t zprefix_len;
    zprefix_len = z2zE3729.ztup1;
    z3zE3598 = zscratch_push_bytes(zencoded_prefix, zprefix_len);
    KILL(zz5listz8z5bv8z9)(&zencoded_prefix);
    goto finish_match_3590;
  }
case_3591: ;
  sail_match_failure("rlp_write_string_prefix<repr:U64,B8,u><bounds:491a746de554142e7d65e0bb42a9e751>");
finish_match_3590: ;
  z8zE773 = z3zE3598;
  KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE3729);
end_function_3592: ;
  return z8zE773;
end_block_exception_3593: ;

  return UNIT;
}

unit zrlp_write_uint_wordzIreprzGU64zCuzKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zw)
{
  unit z8zE774;
  struct ztuple_z8z5listz8z5bv8z9zCz0z5u64z9 z2zE3723;
  CREATE(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE3723);
  zminimal_word_byteszIreprzGU64zCz8LB8zCU64z9zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(&z2zE3723, zw);
  unit z3zE3589;
  {
    zz5listz8z5bv8z9 zbytes;
    CREATE(zz5listz8z5bv8z9)(&zbytes);
    COPY(zz5listz8z5bv8z9)(&zbytes, z2zE3723.ztup0);
    uint64_t zlen;
    zlen = z2zE3723.ztup1;
    z3zE3589 = zrlp_write_bytes(zbytes, zlen);
    KILL(zz5listz8z5bv8z9)(&zbytes);
    goto finish_match_3575;
  }
case_3576: ;
  sail_match_failure("rlp_write_uint_word<repr:U64,u><bounds:4c3f6287b16f25a07ff498da45d6ed37>");
finish_match_3575: ;
  z8zE774 = z3zE3589;
  KILL(ztuple_z8z5listz8z5bv8z9zCz0z5u64z9)(&z2zE3723);
end_function_3577: ;
  return z8zE774;
end_block_exception_3578: ;

  return UNIT;
}

struct zByteSliceFields zsszz_container_byteszIreprzGRByteSliceFieldszCU64zCRByteSliceFieldszKzIboundszG718308cac13a033149f52c385b3cc002zK(struct zByteSliceFields zbytes, uint64_t zminimum)
{
  struct zByteSliceFields z8zE775;
  struct zByteSliceFields zfields;
  zfields = zbytes;
  bool z2zE1092;
  {
    uint64_t z2zE1091;
    z2zE1091 = zfields.zlen;
    {    z2zE1092 = (!(z2zE1091 < zminimum));
    }
  }
  if (z2zE1092) {  z8zE775 = zfields;  } else {
    struct zexception z2zE1093;
    CREATE(zexception)(&z2zE1093);
    zInvalidBlock(&z2zE1093, zInvalidConfig);
    COPY(zexception)(current_exception, z2zE1093);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/ssz/stateless_input.sail:301.8-301.41");
    KILL(zexception)(&z2zE1093);
    goto end_block_exception_1676;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1093);
  }
end_function_1675: ;
  return z8zE775;
end_block_exception_1676: ;
  struct zByteSliceFields z8zE1157 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE1157;
}

struct zByteSliceFields zsszz_container_byteszIreprzGRByteSliceFieldszCU64zCRByteSliceFieldszKzIboundszGd430ac7621e2b62bc50ae9cc272dba03zK(struct zByteSliceFields zbytes, uint64_t zminimum)
{
  struct zByteSliceFields z8zE776;
  struct zByteSliceFields zfields;
  zfields = zbytes;
  bool z2zE1092;
  {
    uint64_t z2zE1091;
    z2zE1091 = zfields.zlen;
    {    z2zE1092 = (!(z2zE1091 < zminimum));
    }
  }
  if (z2zE1092) {  z8zE776 = zfields;  } else {
    struct zexception z2zE1093;
    CREATE(zexception)(&z2zE1093);
    zInvalidBlock(&z2zE1093, zInvalidConfig);
    COPY(zexception)(current_exception, z2zE1093);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/ssz/stateless_input.sail:301.8-301.41");
    KILL(zexception)(&z2zE1093);
    goto end_block_exception_1676;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1093);
  }
end_function_1675: ;
  return z8zE776;
end_block_exception_1676: ;
  struct zByteSliceFields z8zE1158 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE1158;
}

struct zByteSliceFields zsszz_fixed_list_atzIreprzGRBoundedSszzListRefzCU64zCU64zCRByteSliceFieldszKzIboundszG09801315d578666d9a3687b217e19ea1zK(struct zBoundedSszzListRef zitems, uint64_t zindex, uint64_t zitem_sizze)
{
  struct zByteSliceFields z8zE777;
  struct zByteSliceFields zbytes;
  zbytes = zitems.zbytes;
  bool z2zE1102;
  {
    uint64_t z2zE1101;
    z2zE1101 = zitems.zcount;
    {    z2zE1102 = (!(zindex < z2zE1101));
    }
  }
  unit z3zE1224;
  if (z2zE1102) {
    struct zexception z2zE1103;
    CREATE(zexception)(&z2zE1103);
    zInvalidBlock(&z2zE1103, zInvalidConfig);
    COPY(zexception)(current_exception, z2zE1103);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/ssz/stateless_input.sail:231.8-231.41");
    KILL(zexception)(&z2zE1103);
    goto end_block_exception_1680;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1103);
  } else {  z3zE1224 = UNIT;  }
  uint64_t zwidth_value;
  zwidth_value = zitem_sizze;
  sail_u128 zoffset_value;
  {
    sail_u128 z3zE3826;
    z3zE3826 = u128_of_u64(zindex);
    zoffset_value = u128_mul_u64(z3zE3826, zwidth_value);
  }
  uint64_t zitems_length;
  zitems_length = zbytes.zlen;
  bool z2zE1105;
  {
    sail_u128 z2zE1104;
    {    z2zE1104 = u128_add_u64(zoffset_value, zwidth_value);
    }
    {
      sail_u128 z3zE3827;
      z3zE3827 = u128_of_u64(zitems_length);
      z2zE1105 = (!u128_lt(z3zE3827, z2zE1104));
    }
  }
  if (z2zE1105) {
    {
      uint64_t z3zE3872;
      z3zE3872 = u128_to_u64(zoffset_value);
      z8zE777 = zsub_slice(zbytes, z3zE3872, zwidth_value);
    }
  } else {
    struct zexception z2zE1106;
    CREATE(zexception)(&z2zE1106);
    zInvalidBlock(&z2zE1106, zInvalidConfig);
    COPY(zexception)(current_exception, z2zE1106);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/ssz/stateless_input.sail:239.8-239.41");
    KILL(zexception)(&z2zE1106);
    goto end_block_exception_1680;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1106);
  }
end_function_1679: ;
  return z8zE777;
end_block_exception_1680: ;
  struct zByteSliceFields z8zE1159 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE1159;
}

struct zByteSliceFields zsszz_list_atzIreprzGRBoundedSszzListRefzCU64zCRByteSliceFieldszKzIboundszG491a746de554142e7d65e0bb42a9e751zK(struct zBoundedSszzListRef zitems, uint64_t zindex)
{
  struct zByteSliceFields z8zE778;
  struct zByteSliceFields zbytes;
  zbytes = zitems.zbytes;
  uint64_t zcount;
  zcount = zitems.zcount;
  bool z2zE1107;
  {    z2zE1107 = (!(zindex < zcount));
  }
  unit z3zE1225;
  if (z2zE1107) {
    struct zexception z2zE1108;
    CREATE(zexception)(&z2zE1108);
    zInvalidBlock(&z2zE1108, zInvalidConfig);
    COPY(zexception)(current_exception, z2zE1108);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/ssz/stateless_input.sail:193.8-193.41");
    KILL(zexception)(&z2zE1108);
    goto end_block_exception_1682;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1108);
  } else {  z3zE1225 = UNIT;  }
  uint64_t znext_index;
  {
    sail_u128 z3zE3824;
    z3zE3824 = u128_of_u64(zindex);
    sail_u128 z3zE3825;
    z3zE3825 = u128_add_u64(z3zE3824, UINT64_C(1));
    znext_index = u128_to_u64(z3zE3825);
  }
  uint64_t zstart;
  {
    uint64_t z2zE1123;
    {
      uint64_t z2zE1122;
      z2zE1122 = zsszz_offset_table_position(zindex);
      z2zE1123 = zsszz_u32_at(zbytes, z2zE1122);
    }
    zstart = zsszz_offset_to_source_pointer(z2zE1123);
  }
  uint64_t zstop;
  {
    bool z2zE1119;
    {
      uint64_t z2zE1118;
      z2zE1118 = zitems.zcount;
      z2zE1119 = (znext_index < z2zE1118);
    }
    if (z2zE1119) {
      uint64_t z2zE1121;
      {
        uint64_t z2zE1120;
        z2zE1120 = zsszz_offset_table_position(znext_index);
        z2zE1121 = zsszz_u32_at(zbytes, z2zE1120);
      }
      zstop = zsszz_offset_to_source_pointer(z2zE1121);
    } else {  zstop = zbytes.zlen;  }
  }
  uint64_t zstart_value;
  zstart_value = zstart;
  uint64_t zstop_value;
  zstop_value = zstop;
  uint64_t zitems_length;
  zitems_length = zbytes.zlen;
  bool z2zE1109;
  z2zE1109 = (!(zstop_value < zstart_value));
  if (z2zE1109) {
    bool z2zE1110;
    z2zE1110 = (!(zitems_length < zstop_value));
    if (z2zE1110) {
      uint64_t zitem_length;
      {    zitem_length = (zstop_value - zstart_value);
      }
      bool z2zE1114;
      {
        bool z2zE1113;
        {
          uint64_t z2zE1111;
          z2zE1111 = zitems.zmax_item_length;
          z2zE1113 = (z2zE1111 != UINT64_C(0));
        }
        bool z3zE1226;
        if (z2zE1113) {
          uint64_t z2zE1112;
          z2zE1112 = zitems.zmax_item_length;
          z3zE1226 = (z2zE1112 < zitem_length);
        } else {  z3zE1226 = false;  }
        z2zE1114 = z3zE1226;
      }
      unit z3zE1227;
      if (z2zE1114) {
        struct zexception z2zE1115;
        CREATE(zexception)(&z2zE1115);
        zInvalidBlock(&z2zE1115, zInvalidConfig);
        COPY(zexception)(current_exception, z2zE1115);
        have_exception = true;
        COPY(sail_string)(throw_location, "sail/lib/ssz/stateless_input.sail:209.16-209.49");
        KILL(zexception)(&z2zE1115);
        goto end_block_exception_1682;
        /* unreachable after throw */
        KILL(zexception)(&z2zE1115);
      } else {  z3zE1227 = UNIT;  }
      z8zE778 = zsub_slice(zbytes, zstart, zitem_length);
    } else {
      struct zexception z2zE1116;
      CREATE(zexception)(&z2zE1116);
      zInvalidBlock(&z2zE1116, zInvalidConfig);
      COPY(zexception)(current_exception, z2zE1116);
      have_exception = true;
      COPY(sail_string)(throw_location, "sail/lib/ssz/stateless_input.sail:213.12-213.45");
      KILL(zexception)(&z2zE1116);
      goto end_block_exception_1682;
      /* unreachable after throw */
      KILL(zexception)(&z2zE1116);
    }
  } else {
    struct zexception z2zE1117;
    CREATE(zexception)(&z2zE1117);
    zInvalidBlock(&z2zE1117, zInvalidConfig);
    COPY(zexception)(current_exception, z2zE1117);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/lib/ssz/stateless_input.sail:216.8-216.41");
    KILL(zexception)(&z2zE1117);
    goto end_block_exception_1682;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1117);
  }
end_function_1681: ;
  return z8zE778;
end_block_exception_1682: ;
  struct zByteSliceFields z8zE1160 = { .zlen = UINT64_C(0xdeadc0de), .zoff = UINT64_C(0xdeadc0de), .zsource = ((enum zByteSource)0) };
  return z8zE1160;
}

uint64_t zstate_gas_spill_addzIreprzGU64zCI128zCU64zKzIboundszG1c0bd00f9b9bd1c00190715feb145f1dzK(uint64_t zleft, __int128 zright)
{
  uint64_t z8zE779;
  bool z2zE2774;
  {
    uint64_t z2zE2773;
    z2zE2773 = zstate_gas_spill_room(zleft);
    z2zE2774 = (!(z2zE2773 < zright));
  }
  if (z2zE2774) {
    {
      __int128 z3zE3835;
      z3zE3835 = (__int128)(zleft);
      __int128 z3zE3836;
      z3zE3836 = (zright + z3zE3835);
      z8zE779 = (uint64_t)(z3zE3836);
    }
  } else {
    struct zexception z2zE2775;
    CREATE(zexception)(&z2zE2775);
    zInvalidBlock(&z2zE2775, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE2775);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/gas.sail:454.8-454.44");
    KILL(zexception)(&z2zE2775);
    goto end_block_exception_2873;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2775);
  }
end_function_2872: ;
  return z8zE779;
end_block_exception_2873: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zstate_gas_spill_addzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(uint64_t zleft, uint64_t zright)
{
  uint64_t z8zE780;
  bool z2zE2774;
  {
    uint64_t z2zE2773;
    z2zE2773 = zstate_gas_spill_room(zleft);
    {    z2zE2774 = (!(z2zE2773 < zright));
    }
  }
  if (z2zE2774) {
    {
      sail_u128 z3zE3799;
      z3zE3799 = u128_of_u64(zleft);
      sail_u128 z3zE3800;
      z3zE3800 = u128_add_u64(z3zE3799, zright);
      z8zE780 = u128_to_u64(z3zE3800);
    }
  } else {
    struct zexception z2zE2775;
    CREATE(zexception)(&z2zE2775);
    zInvalidBlock(&z2zE2775, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE2775);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/gas.sail:454.8-454.44");
    KILL(zexception)(&z2zE2775);
    goto end_block_exception_2873;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2775);
  }
end_function_2872: ;
  return z8zE780;
end_block_exception_2873: ;

  return UINT64_C(0xdeadc0de);
}

sail_fixed_bytes_20 zto_bytes_lezIreprzGU64zCR__sail_c_repr_u256zCR__sail_c_repr_fixed_byteszIC20zKzKzIboundszG83613fe416f377787be94d88c23f4a78zK(uint64_t zn, sail_u256 zb)
{
  sail_fixed_bytes_20 z8zE781;
  z8zE781 = fixed_bytes_20_from_u256(zb);
  return z8zE781;
}

sail_fixed_bytes_32 zto_bytes_lezIreprzGU64zCR__sail_c_repr_u256zCR__sail_c_repr_fixed_byteszIC32zKzKzIboundszG78c09f65c90525db636e344fe928bc0bzK(uint64_t zn, sail_u256 zb)
{
  sail_fixed_bytes_32 z8zE782;
  z8zE782 = fixed_bytes_32_from_u256(zb);
  return z8zE782;
}

uint64_t zvalidated_gas_subzIreprzGU64zCU64zCU64zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(uint64_t zleft, uint64_t zright)
{
  uint64_t z8zE783;
  bool z2zE1981;
  {    z2zE1981 = (!(zleft < zright));
  }
  if (z2zE1981) {
    {
      __int128 z3zE3808;
      z3zE3808 = (__int128)(zleft);
      __int128 z3zE3809;
      z3zE3809 = (__int128)(zright);
      __int128 z3zE3810;
      z3zE3810 = (z3zE3808 - z3zE3809);
      z8zE783 = (uint64_t)(z3zE3810);
    }
  } else {
    struct zexception z2zE1982;
    CREATE(zexception)(&z2zE1982);
    zInvalidBlock(&z2zE1982, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE1982);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/transaction.sail:316.8-316.44");
    KILL(zexception)(&z2zE1982);
    goto end_block_exception_2140;
    /* unreachable after throw */
    KILL(zexception)(&z2zE1982);
  }
end_function_2139: ;
  return z8zE783;
end_block_exception_2140: ;

  return UINT64_C(0xdeadc0de);
}

__int128 zvalidated_refund_addzIreprzGI128zCI64zCI128zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(__int128 zleft, int64_t zright)
{
  __int128 z8zE784;
  __int128 ztotal;
  {    ztotal = (zleft + zright);
  }
  bool z2zE2871;
  {
    bool z2zE2870;
    z2zE2870 = (!(ztotal < (-((__int128)((((unsigned __int128)UINT64_C(198)) << 64) | UINT64_C(18446744073709551416))) - 1)));
    bool z3zE2971;
    if (z2zE2870) {
      z3zE2971 = (!(((__int128)((((unsigned __int128)UINT64_C(198)) << 64) | UINT64_C(18446744073709551417))) < ztotal));
    } else {  z3zE2971 = false;  }
    z2zE2871 = z3zE2971;
  }
  if (z2zE2871) {  z8zE784 = ztotal;  } else {
    struct zexception z2zE2872;
    CREATE(zexception)(&z2zE2872);
    zInvalidBlock(&z2zE2872, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE2872);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/machine.sail:95.8-95.44");
    KILL(zexception)(&z2zE2872);
    goto end_block_exception_3000;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2872);
  }
end_function_2999: ;
  return z8zE784;
end_block_exception_3000: ;

  return ((__int128)INT64_C(0xdeadc0de));
}

__int128 zvalidated_refund_addzIreprzGI128zCU64zCI128zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(__int128 zleft, uint64_t zright)
{
  __int128 z8zE785;
  __int128 ztotal;
  {    ztotal = (zleft + zright);
  }
  bool z2zE2871;
  {
    bool z2zE2870;
    z2zE2870 = (!(ztotal < (-((__int128)((((unsigned __int128)UINT64_C(198)) << 64) | UINT64_C(18446744073709551416))) - 1)));
    bool z3zE2971;
    if (z2zE2870) {
      z3zE2971 = (!(((__int128)((((unsigned __int128)UINT64_C(198)) << 64) | UINT64_C(18446744073709551417))) < ztotal));
    } else {  z3zE2971 = false;  }
    z2zE2871 = z3zE2971;
  }
  if (z2zE2871) {  z8zE785 = ztotal;  } else {
    struct zexception z2zE2872;
    CREATE(zexception)(&z2zE2872);
    zInvalidBlock(&z2zE2872, zExecutionInvalid);
    COPY(zexception)(current_exception, z2zE2872);
    have_exception = true;
    COPY(sail_string)(throw_location, "sail/evm/machine.sail:95.8-95.44");
    KILL(zexception)(&z2zE2872);
    goto end_block_exception_3000;
    /* unreachable after throw */
    KILL(zexception)(&z2zE2872);
  }
end_function_2999: ;
  return z8zE785;
end_block_exception_3000: ;

  return ((__int128)INT64_C(0xdeadc0de));
}

sail_u256 zvalidated_word_productzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256 zvalue, uint64_t zfactor)
{
  sail_u256 z8zE786;
  {
    z8zE786 = zblob_word_mulzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zvalue, zfactor);
    if (have_exception) {  goto end_block_exception_2144;  }
  }
end_function_2143: ;
  return z8zE786;
end_block_exception_2144: ;

  return u256_zero();
}

sail_u256 zword_add_wordzIreprzGR__sail_c_repr_u256zCR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256 zleft, sail_u128 zright)
{
  sail_u256 z8zE787;
  sail_u256 z2zE4073;
  {    z2zE4073 = u256_add_u128(zleft, zright);
  }
  z8zE787 = zU256(z2zE4073);
end_function_3956: ;
  return z8zE787;
end_block_exception_3957: ;

  return u256_zero();
}

sail_u256 zword_add_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256 zleft, uint64_t zright)
{
  sail_u256 z8zE788;
  sail_u256 z2zE4073;
  {    z2zE4073 = u256_add_u64(zleft, zright);
  }
  z8zE788 = zU256(z2zE4073);
end_function_3956: ;
  return z8zE788;
end_block_exception_3957: ;

  return u256_zero();
}

uint64_t zword_bit_lengthzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zvalue)
{
  uint64_t z8zE789;
  uint64_t zlimb3;
  {
    uint64_t z2zE4034;
    z2zE4034 = (safe_rshift(UINT64_MAX, 64 - 64) & safe_rshift(zvalue, UINT64_C(192)));
    zlimb3 = ((uint64_t) z2zE4034);
  }
  bool z2zE4024;
  z2zE4024 = (zlimb3 != UINT64_C(0));
  if (z2zE4024) {
    uint64_t z2zE4025;
    z2zE4025 = zu64_bit_length(zlimb3);
    {    z8zE789 = (UINT64_C(192) + z2zE4025);
    }
  } else {
    uint64_t zlimb2;
    {
      uint64_t z2zE4033;
      z2zE4033 = (safe_rshift(UINT64_MAX, 64 - 64) & safe_rshift(zvalue, UINT64_C(128)));
      zlimb2 = ((uint64_t) z2zE4033);
    }
    bool z2zE4026;
    z2zE4026 = (zlimb2 != UINT64_C(0));
    if (z2zE4026) {
      uint64_t z2zE4027;
      z2zE4027 = zu64_bit_length(zlimb2);
      {    z8zE789 = (UINT64_C(128) + z2zE4027);
      }
    } else {
      uint64_t zlimb1;
      {
        uint64_t z2zE4032;
        z2zE4032 = (safe_rshift(UINT64_MAX, 64 - 64) & safe_rshift(zvalue, UINT64_C(64)));
        zlimb1 = ((uint64_t) z2zE4032);
      }
      bool z2zE4028;
      z2zE4028 = (zlimb1 != UINT64_C(0));
      if (z2zE4028) {
        uint64_t z2zE4029;
        z2zE4029 = zu64_bit_length(zlimb1);
        {    z8zE789 = (UINT64_C(64) + z2zE4029);
        }
      } else {
        uint64_t z2zE4031;
        {
          uint64_t z2zE4030;
          z2zE4030 = (safe_rshift(UINT64_MAX, 64 - 64) & safe_rshift(zvalue, UINT64_C(0)));
          z2zE4031 = ((uint64_t) z2zE4030);
        }
        z8zE789 = zu64_bit_length(z2zE4031);
      }
    }
  }
end_function_3920: ;
  return z8zE789;
end_block_exception_3921: ;

  return UINT64_C(0xdeadc0de);
}

uint64_t zword_byte_lengthzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zvalue)
{
  uint64_t z8zE790;
  uint64_t zbit_length;
  zbit_length = zword_bit_lengthzIreprzGU64zCU64zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zvalue);
  bool z2zE4012;
  z2zE4012 = (zbit_length == UINT64_C(0));
  if (z2zE4012) {  z8zE790 = UINT64_C(0);  } else {
    uint64_t z2zE4013;
    {    z2zE4013 = (zbit_length + UINT64_C(7));
    }
    {    z8zE790 = (z2zE4013 / UINT64_C(8));
    }
  }
end_function_3906: ;
  return z8zE790;
end_block_exception_3907: ;

  return UINT64_C(0xdeadc0de);
}

sail_u256 zword_div_wordzIreprzGR__sail_c_repr_u128zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u128 zdividend, uint64_t zdivisor)
{
  sail_u256 z8zE791;
  bool z2zE4021;
  z2zE4021 = (zdivisor == UINT64_C(0));
  if (z2zE4021) {  z8zE791 = zWORD_ZERO;  } else {
    sail_u256 z2zE4022;
    z2zE4022 = u256_of_u128(u128_div_u64(zdividend, zdivisor));
    z8zE791 = zU256(z2zE4022);
  }
end_function_3916: ;
  return z8zE791;
end_block_exception_3917: ;

  return u256_zero();
}

sail_u256 zword_div_wordzIreprzGR__sail_c_repr_u256zCR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256 zdividend, sail_u128 zdivisor)
{
  sail_u256 z8zE792;
  bool z2zE4021;
  z2zE4021 = u128_eq_u64(zdivisor, UINT64_C(0));
  if (z2zE4021) {  z8zE792 = zWORD_ZERO;  } else {
    sail_u256 z2zE4022;
    z2zE4022 = u256_div_u128(zdividend, zdivisor);
    z8zE792 = zU256(z2zE4022);
  }
end_function_3916: ;
  return z8zE792;
end_block_exception_3917: ;

  return u256_zero();
}

sail_u256 zword_div_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256 zdividend, uint64_t zdivisor)
{
  sail_u256 z8zE793;
  bool z2zE4021;
  z2zE4021 = (zdivisor == UINT64_C(0));
  if (z2zE4021) {  z8zE793 = zWORD_ZERO;  } else {
    sail_u256 z2zE4022;
    z2zE4022 = u256_div_u64(zdividend, zdivisor);
    z8zE793 = zU256(z2zE4022);
  }
end_function_3916: ;
  return z8zE793;
end_block_exception_3917: ;

  return u256_zero();
}

bool zword_greater_than_wordzIreprzGR__sail_c_repr_u256zCU64zCozKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256 zleft, uint64_t zright)
{
  bool z8zE794;
  z8zE794 = u64_lt_u256(zright, zleft);
end_function_3912: ;
  return z8zE794;
end_block_exception_3913: ;

  return false;
}

bool zword_is_zzerozIreprzGR__sail_c_repr_u128zCozKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(sail_u128 zw)
{
  bool z8zE795;
  z8zE795 = u256_eq_u128(zWORD_ZERO, zw);
end_function_3936: ;
  return z8zE795;
end_block_exception_3937: ;

  return false;
}

bool zword_is_zzerozIreprzGU64zCozKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zw)
{
  bool z8zE796;
  z8zE796 = u256_eq_u64(zWORD_ZERO, zw);
end_function_3936: ;
  return z8zE796;
end_block_exception_3937: ;

  return false;
}

uint64_t zword_low_bytezIreprzGU64zCB8zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zvalue)
{
  uint64_t z8zE797;
  z8zE797 = (safe_rshift(UINT64_MAX, 64 - 8) & safe_rshift(zvalue, UINT64_C(0)));
end_function_3942: ;
  return z8zE797;
end_block_exception_3943: ;

  return UINT64_C(0xdeadc0de);
}

sail_u256 zword_mod_wordzIreprzGR__sail_c_repr_u256zCR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256 zdividend, sail_u128 zdivisor)
{
  sail_u256 z8zE798;
  bool z2zE4019;
  z2zE4019 = u128_eq_u64(zdivisor, UINT64_C(0));
  if (z2zE4019) {  z8zE798 = zWORD_ZERO;  } else {
    sail_u256 z2zE4020;
    z2zE4020 = u256_mod_u128(zdividend, zdivisor);
    z8zE798 = zU256(z2zE4020);
  }
end_function_3914: ;
  return z8zE798;
end_block_exception_3915: ;

  return u256_zero();
}

sail_u256 zword_mod_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256 zdividend, uint64_t zdivisor)
{
  sail_u256 z8zE799;
  bool z2zE4019;
  z2zE4019 = (zdivisor == UINT64_C(0));
  if (z2zE4019) {  z8zE799 = zWORD_ZERO;  } else {
    sail_u256 z2zE4020;
    z2zE4020 = u256_mod_u64(zdividend, zdivisor);
    z8zE799 = zU256(z2zE4020);
  }
end_function_3914: ;
  return z8zE799;
end_block_exception_3915: ;

  return u256_zero();
}

sail_u256 zword_mul_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256 za, uint64_t zb)
{
  sail_u256 z8zE800;
  z8zE800 = u256_mul_u64(za, zb);
end_function_3918: ;
  return z8zE800;
end_block_exception_3919: ;

  return u256_zero();
}

sail_u256 zword_of_nat_byte_countzIreprzGR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(sail_u128 zvalue)
{
  sail_u256 z8zE801;
  bool z2zE3951;
  {    z2zE3951 = true;
  }
  if (z2zE3951) {
    z8zE801 = zU256zIreprzGR__sail_c_repr_u128zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zvalue);
  } else {
    unit z3zE3756;
    z3zE3756 = sail_assert(false, "sail/primitives/quantities.sail:363.20-363.21");
    sail_match_failure("word_of_nat_byte_count<repr:R__sail_c_repr_u128,R__sail_c_repr_u256><bounds:4c3f6287b16f25a07ff498da45d6ed37>");
    /* unreachable after exit */
  }
end_function_3812: ;
  return z8zE801;
end_block_exception_3813: ;

  return u256_zero();
}

sail_u256 zword_of_nat_byte_countzIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(uint64_t zvalue)
{
  sail_u256 z8zE802;
  bool z2zE3951;
  {    z2zE3951 = true;
  }
  if (z2zE3951) {
    z8zE802 = zU256zIreprzGU64zCR__sail_c_repr_u256zKzIboundszG4c3f6287b16f25a07ff498da45d6ed37zK(zvalue);
  } else {
    unit z3zE3756;
    z3zE3756 = sail_assert(false, "sail/primitives/quantities.sail:363.20-363.21");
    sail_match_failure("word_of_nat_byte_count<repr:U64,R__sail_c_repr_u256><bounds:4c3f6287b16f25a07ff498da45d6ed37>");
    /* unreachable after exit */
  }
end_function_3812: ;
  return z8zE802;
end_block_exception_3813: ;

  return u256_zero();
}

sail_u256 zword_sub_wordzIreprzGR__sail_c_repr_u256zCU64zCR__sail_c_repr_u256zKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256 zleft, uint64_t zright)
{
  sail_u256 z8zE803;
  sail_u256 z2zE4071;
  {    z2zE4071 = u256_sub_u64(zleft, zright);
  }
  z8zE803 = zU256(z2zE4071);
end_function_3954: ;
  return z8zE803;
end_block_exception_3955: ;

  return u256_zero();
}

bool zword_ulezIreprzGU64zCR__sail_c_repr_u256zCozKzIboundszG491a746de554142e7d65e0bb42a9e751zK(uint64_t za, sail_u256 zb)
{
  bool z8zE804;
  bool z2zE4051;
  z2zE4051 = zword_ultzIreprzGR__sail_c_repr_u256zCU64zCozKzIboundszG491a746de554142e7d65e0bb42a9e751zK(zb, za);
  z8zE804 = not(z2zE4051);
end_function_3930: ;
  return z8zE804;
end_block_exception_3931: ;

  return false;
}

bool zword_ultzIreprzGR__sail_c_repr_u256zCU64zCozKzIboundszG491a746de554142e7d65e0bb42a9e751zK(sail_u256 za, uint64_t zb)
{
  bool z8zE805;
  z8zE805 = u256_lt_u64(za, zb);
end_function_3932: ;
  return z8zE805;
end_block_exception_3933: ;

  return false;
}

void model_init(void)
{
  setup_rts();
  current_exception = sail_new(struct zexception);
  CREATE(zexception)(current_exception);
  throw_location = sail_new(sail_string);
  CREATE(sail_string)(throw_location);
  create_letbind_0();
  create_letbind_1();
  create_letbind_2();
  create_letbind_3();
  create_letbind_4();
  create_letbind_5();
  create_letbind_6();
  create_letbind_7();
  create_letbind_8();
  create_letbind_9();
  create_letbind_10();
  create_letbind_11();
  create_letbind_12();
  create_letbind_13();
  create_letbind_14();
  create_letbind_15();
  create_letbind_16();
  create_letbind_17();
  create_letbind_18();
  create_letbind_19();
  create_letbind_20();
  create_letbind_21();
  create_letbind_22();
  create_letbind_23();
  create_letbind_24();
  create_letbind_25();
  create_letbind_26();
  create_letbind_27();
  create_letbind_28();
  create_letbind_29();
  create_letbind_30();
  create_letbind_31();
  create_letbind_32();
  create_letbind_33();
  create_letbind_34();
  create_letbind_35();
  create_letbind_36();
  create_letbind_37();
  create_letbind_38();
  create_letbind_39();
  create_letbind_40();
  create_letbind_41();
  create_letbind_42();
  create_letbind_43();
  create_letbind_44();
  create_letbind_45();
  create_letbind_46();
  create_letbind_47();
  create_letbind_48();
  create_letbind_49();
  create_letbind_50();
  create_letbind_51();
  create_letbind_52();
  create_letbind_53();
  create_letbind_54();
  create_letbind_55();
  create_letbind_56();
  create_letbind_57();
  create_letbind_58();
  create_letbind_59();
  create_letbind_60();
  create_letbind_61();
  create_letbind_62();
  create_letbind_63();
  create_letbind_64();
  create_letbind_65();
  create_letbind_66();
  create_letbind_67();
  create_letbind_68();
  create_letbind_69();
  create_letbind_70();
  create_letbind_71();
  create_letbind_72();
  create_letbind_73();
  create_letbind_74();
  create_letbind_75();
  create_letbind_76();
  create_letbind_77();
  create_letbind_78();
  create_letbind_79();
  create_letbind_80();
  create_letbind_81();
  create_letbind_82();
  create_letbind_83();
  create_letbind_84();
  create_letbind_85();
  create_letbind_86();
  create_letbind_87();
  create_letbind_88();
  create_letbind_89();
  create_letbind_90();
  create_letbind_91();
  create_letbind_92();
  create_letbind_93();
  create_letbind_94();
  create_letbind_95();
  create_letbind_96();
  create_letbind_97();
  create_letbind_98();
  create_letbind_99();
  create_letbind_100();
  create_letbind_101();
  create_letbind_102();
  create_letbind_103();
  create_letbind_104();
  create_letbind_105();
  create_letbind_106();
  create_letbind_107();
  create_letbind_108();
  create_letbind_109();
  create_letbind_110();
  create_letbind_111();
  create_letbind_112();
  create_letbind_113();
  create_letbind_114();
  create_letbind_115();
  create_letbind_116();
  create_letbind_117();
  create_letbind_118();
  create_letbind_119();
  create_letbind_120();
  create_letbind_121();
  create_letbind_122();
  create_letbind_123();
  create_letbind_124();
  create_letbind_125();
  create_letbind_126();
  create_letbind_127();
  create_letbind_128();
  create_letbind_129();
  create_letbind_130();
  create_letbind_131();
  create_letbind_132();
  create_letbind_133();
  create_letbind_134();
  create_letbind_135();
  create_letbind_136();
  create_letbind_137();
  create_letbind_138();
  create_letbind_139();
  create_letbind_140();
  create_letbind_141();
  create_letbind_142();
  create_letbind_143();
  create_letbind_144();
  create_letbind_145();
  create_letbind_146();
  create_letbind_147();
  create_letbind_148();
  create_letbind_149();
  create_letbind_150();
  create_letbind_151();
  create_letbind_152();
  create_letbind_153();
  create_letbind_154();
  create_letbind_155();
  create_letbind_156();
  create_letbind_157();
  create_letbind_158();
  create_letbind_159();
  create_letbind_160();
  create_letbind_161();
  create_letbind_162();
  create_letbind_163();
  create_letbind_164();
  create_letbind_165();
  create_letbind_166();
  create_letbind_167();
  create_letbind_168();
  create_letbind_169();
  create_letbind_170();
  create_letbind_171();
  create_letbind_172();
  create_letbind_173();
  create_letbind_174();
  create_letbind_175();
  create_letbind_176();
  create_letbind_177();
  create_letbind_178();
  create_letbind_179();
  create_letbind_180();
  create_letbind_181();
  create_letbind_182();
  create_letbind_183();
  create_letbind_184();
  create_letbind_185();
  create_letbind_186();
  create_letbind_187();
  create_letbind_188();
  create_letbind_189();
  create_letbind_190();
  create_letbind_191();
  create_letbind_192();
  create_letbind_193();
  create_letbind_194();
  create_letbind_195();
  create_letbind_196();
  create_letbind_197();
  create_letbind_198();
  create_letbind_199();
  create_letbind_200();
  create_letbind_201();
  create_letbind_202();
  create_letbind_203();
  create_letbind_204();
  create_letbind_205();
  create_letbind_206();
  create_letbind_207();
  create_letbind_208();
  create_letbind_209();
  create_letbind_210();
  create_letbind_211();
  create_letbind_212();
  create_letbind_213();
  create_letbind_214();
  create_letbind_215();
  create_letbind_216();
  create_letbind_217();
  create_letbind_218();
  create_letbind_219();
  create_letbind_220();
  create_letbind_221();
  create_letbind_222();
  create_letbind_223();
  create_letbind_224();
  create_letbind_225();
  create_letbind_226();
  create_letbind_227();
  create_letbind_228();
  create_letbind_229();
  create_letbind_230();
  create_letbind_231();
  create_letbind_232();
  create_letbind_233();
  create_letbind_234();
  create_letbind_235();
  create_letbind_236();
  create_letbind_237();
  create_letbind_238();
  create_letbind_239();
  create_letbind_240();
  create_letbind_241();
  create_letbind_242();
  create_letbind_243();
  create_letbind_244();
  create_letbind_245();
  create_letbind_246();
  create_letbind_247();
  create_letbind_248();
  create_letbind_249();
  create_letbind_250();
  create_letbind_251();
  create_letbind_252();
  create_letbind_253();
  create_letbind_254();
  create_letbind_255();
  create_letbind_256();
  create_letbind_257();
  create_letbind_258();
  create_letbind_259();
  create_letbind_260();
  create_letbind_261();
  create_letbind_262();
  create_letbind_263();
  create_letbind_264();
  create_letbind_265();
  create_letbind_266();
  create_letbind_267();
  create_letbind_268();
  create_letbind_269();
  create_letbind_270();
  create_letbind_271();
  create_letbind_272();
  create_letbind_273();
  create_letbind_274();
  create_letbind_275();
  create_letbind_276();
  create_letbind_277();
  create_letbind_278();
  create_letbind_279();
  create_letbind_280();
  create_letbind_281();
  create_letbind_282();
  create_letbind_283();
  create_letbind_284();
  create_letbind_285();
  create_letbind_286();
  create_letbind_287();
  create_letbind_288();
  create_letbind_289();
  create_letbind_290();
  create_letbind_291();
  create_letbind_292();
  create_letbind_293();
  create_letbind_294();
  create_letbind_295();
  create_letbind_296();
  create_letbind_297();
  create_letbind_298();
  create_letbind_299();
  create_letbind_300();
  create_letbind_301();
  create_letbind_302();
  create_letbind_303();
  create_letbind_304();
  create_letbind_305();
  create_letbind_306();
  create_letbind_307();
  create_letbind_308();
  create_letbind_309();
  create_letbind_310();
  create_letbind_311();
  create_letbind_312();
  create_letbind_313();
  create_letbind_314();
  create_letbind_315();
  create_letbind_316();
  create_letbind_317();
  create_letbind_318();
  create_letbind_319();
  create_letbind_320();
  create_letbind_321();
  create_letbind_322();
  create_letbind_323();
  create_letbind_324();
  create_letbind_325();
  create_letbind_326();
  create_letbind_327();
  create_letbind_328();
  create_letbind_329();
  create_letbind_330();
  create_letbind_331();
  create_letbind_332();
  create_letbind_333();
  create_letbind_334();
  create_letbind_335();
  create_letbind_336();
  create_letbind_337();
  create_letbind_338();
  create_letbind_339();
  create_letbind_340();
  create_letbind_341();
  create_letbind_342();
  create_letbind_343();
  create_letbind_344();
  create_letbind_345();
  create_letbind_346();
  create_letbind_347();
  create_letbind_348();
  create_letbind_349();
  create_letbind_350();
  create_letbind_351();
  create_letbind_352();
  create_letbind_353();
  create_letbind_354();
  create_letbind_355();
  create_letbind_356();
  create_letbind_357();
  create_letbind_358();
  create_letbind_359();
  create_letbind_360();
  create_letbind_361();
  zscratch_arena = zbyte_slice(zScratchSource, UINT64_C(0), UINT64_C(0));
  zk_parent_state_root = zZERO_HASH;
  zk_n_headers = UINT64_C(0);
  zk_chain_id = UINT64_C(1);
  zk_fork = zAmsterdam;
  struct zBlobSchedule z3zE126;
  z3zE126.zbase_fee_update_fraction = UINT64_C(11684671);
  z3zE126.zmax = UINT64_C(21);
  z3zE126.ztarget = UINT64_C(14);
  zk_blob_schedule = z3zE126;
  struct zBlockHeader z3zE127;
  z3zE127.zbase_fee = zZERO_WORD;
  z3zE127.zblob_gas_used = UINT64_C(0);
  z3zE127.zexcess_blob_gas = UINT64_C(0);
  z3zE127.zextra_data = zEMPTY_SLICE;
  z3zE127.zfee_recipient = zZERO_ADDRESS;
  z3zE127.zgas_limit = UINT64_C(0);
  z3zE127.zgas_used = UINT64_C(0);
  z3zE127.zlogs_bloom = zEMPTY_LOGS_BLOOM;
  z3zE127.znumber = UINT64_C(0);
  z3zE127.zparent_beacon_block_root = zZERO_HASH;
  z3zE127.zparent_hash = zZERO_HASH;
  z3zE127.zprev_randao = zZERO_WORD;
  z3zE127.zreceipts_root = zZERO_HASH;
  z3zE127.zslot_number = UINT64_C(0);
  z3zE127.zstate_root = zZERO_HASH;
  z3zE127.ztimestamp = UINT64_C(0);
  zk_header = z3zE127;
  struct zTxEnv z3zE128;
  z3zE128.zblob_hashes = zEMPTY_BLOB_HASHES;
  z3zE128.zgas_price = zZERO_WORD;
  z3zE128.zorigin = zZERO_ADDRESS;
  zk_tx = z3zE128;
  zpc = UINT64_C(0);
  zgas_remaining = zGAS_ZERO;
  zstate_gas_remaining = zGAS_ZERO;
  zstate_gas_spilled = zSTATE_GAS_SPILL_ZERO;
  zframe_refund = zGAS_REFUND_ZERO;
  CREATE(zFrameStatus)(&zframe_status);
  zRunning(&zframe_status, UNIT);
  zmessage = zDEFAULT_MESSAGE;
  zcall_depth = UINT64_C(0);
  CREATE(zz5vecz8z5unionz0zzFrameContinuationz9)(&zframe_stack);
  fast_unsigned_vector_init_zz5vecz8z5unionz0zzFrameContinuationz9(&zframe_stack, UINT64_C(1024), zDEFAULT_FRAME_CONTINUATION);
  zframe_stack_top = UINT64_C(0);
  zframe_code = zEMPTY_CODE;
  zcalldata = zEMPTY_SLICE;
  zreturndata = zEMPTY_SLICE;
  zevm_memory = zbyte_slice(zEvmMemorySource, UINT64_C(0), UINT64_C(0));
  zinitializze_registers(UNIT);
}

void model_fini(void)
{
  KILL(zFrameStatus)(&zframe_status);
  KILL(zz5vecz8z5unionz0zzFrameContinuationz9)(&zframe_stack);
  kill_letbind_361();
  kill_letbind_360();
  kill_letbind_359();
  kill_letbind_358();
  kill_letbind_357();
  kill_letbind_356();
  kill_letbind_355();
  kill_letbind_354();
  kill_letbind_353();
  kill_letbind_352();
  kill_letbind_351();
  kill_letbind_350();
  kill_letbind_349();
  kill_letbind_348();
  kill_letbind_347();
  kill_letbind_346();
  kill_letbind_345();
  kill_letbind_344();
  kill_letbind_343();
  kill_letbind_342();
  kill_letbind_341();
  kill_letbind_340();
  kill_letbind_339();
  kill_letbind_338();
  kill_letbind_337();
  kill_letbind_336();
  kill_letbind_335();
  kill_letbind_334();
  kill_letbind_333();
  kill_letbind_332();
  kill_letbind_331();
  kill_letbind_330();
  kill_letbind_329();
  kill_letbind_328();
  kill_letbind_327();
  kill_letbind_326();
  kill_letbind_325();
  kill_letbind_324();
  kill_letbind_323();
  kill_letbind_322();
  kill_letbind_321();
  kill_letbind_320();
  kill_letbind_319();
  kill_letbind_318();
  kill_letbind_317();
  kill_letbind_316();
  kill_letbind_315();
  kill_letbind_314();
  kill_letbind_313();
  kill_letbind_312();
  kill_letbind_311();
  kill_letbind_310();
  kill_letbind_309();
  kill_letbind_308();
  kill_letbind_307();
  kill_letbind_306();
  kill_letbind_305();
  kill_letbind_304();
  kill_letbind_303();
  kill_letbind_302();
  kill_letbind_301();
  kill_letbind_300();
  kill_letbind_299();
  kill_letbind_298();
  kill_letbind_297();
  kill_letbind_296();
  kill_letbind_295();
  kill_letbind_294();
  kill_letbind_293();
  kill_letbind_292();
  kill_letbind_291();
  kill_letbind_290();
  kill_letbind_289();
  kill_letbind_288();
  kill_letbind_287();
  kill_letbind_286();
  kill_letbind_285();
  kill_letbind_284();
  kill_letbind_283();
  kill_letbind_282();
  kill_letbind_281();
  kill_letbind_280();
  kill_letbind_279();
  kill_letbind_278();
  kill_letbind_277();
  kill_letbind_276();
  kill_letbind_275();
  kill_letbind_274();
  kill_letbind_273();
  kill_letbind_272();
  kill_letbind_271();
  kill_letbind_270();
  kill_letbind_269();
  kill_letbind_268();
  kill_letbind_267();
  kill_letbind_266();
  kill_letbind_265();
  kill_letbind_264();
  kill_letbind_263();
  kill_letbind_262();
  kill_letbind_261();
  kill_letbind_260();
  kill_letbind_259();
  kill_letbind_258();
  kill_letbind_257();
  kill_letbind_256();
  kill_letbind_255();
  kill_letbind_254();
  kill_letbind_253();
  kill_letbind_252();
  kill_letbind_251();
  kill_letbind_250();
  kill_letbind_249();
  kill_letbind_248();
  kill_letbind_247();
  kill_letbind_246();
  kill_letbind_245();
  kill_letbind_244();
  kill_letbind_243();
  kill_letbind_242();
  kill_letbind_241();
  kill_letbind_240();
  kill_letbind_239();
  kill_letbind_238();
  kill_letbind_237();
  kill_letbind_236();
  kill_letbind_235();
  kill_letbind_234();
  kill_letbind_233();
  kill_letbind_232();
  kill_letbind_231();
  kill_letbind_230();
  kill_letbind_229();
  kill_letbind_228();
  kill_letbind_227();
  kill_letbind_226();
  kill_letbind_225();
  kill_letbind_224();
  kill_letbind_223();
  kill_letbind_222();
  kill_letbind_221();
  kill_letbind_220();
  kill_letbind_219();
  kill_letbind_218();
  kill_letbind_217();
  kill_letbind_216();
  kill_letbind_215();
  kill_letbind_214();
  kill_letbind_213();
  kill_letbind_212();
  kill_letbind_211();
  kill_letbind_210();
  kill_letbind_209();
  kill_letbind_208();
  kill_letbind_207();
  kill_letbind_206();
  kill_letbind_205();
  kill_letbind_204();
  kill_letbind_203();
  kill_letbind_202();
  kill_letbind_201();
  kill_letbind_200();
  kill_letbind_199();
  kill_letbind_198();
  kill_letbind_197();
  kill_letbind_196();
  kill_letbind_195();
  kill_letbind_194();
  kill_letbind_193();
  kill_letbind_192();
  kill_letbind_191();
  kill_letbind_190();
  kill_letbind_189();
  kill_letbind_188();
  kill_letbind_187();
  kill_letbind_186();
  kill_letbind_185();
  kill_letbind_184();
  kill_letbind_183();
  kill_letbind_182();
  kill_letbind_181();
  kill_letbind_180();
  kill_letbind_179();
  kill_letbind_178();
  kill_letbind_177();
  kill_letbind_176();
  kill_letbind_175();
  kill_letbind_174();
  kill_letbind_173();
  kill_letbind_172();
  kill_letbind_171();
  kill_letbind_170();
  kill_letbind_169();
  kill_letbind_168();
  kill_letbind_167();
  kill_letbind_166();
  kill_letbind_165();
  kill_letbind_164();
  kill_letbind_163();
  kill_letbind_162();
  kill_letbind_161();
  kill_letbind_160();
  kill_letbind_159();
  kill_letbind_158();
  kill_letbind_157();
  kill_letbind_156();
  kill_letbind_155();
  kill_letbind_154();
  kill_letbind_153();
  kill_letbind_152();
  kill_letbind_151();
  kill_letbind_150();
  kill_letbind_149();
  kill_letbind_148();
  kill_letbind_147();
  kill_letbind_146();
  kill_letbind_145();
  kill_letbind_144();
  kill_letbind_143();
  kill_letbind_142();
  kill_letbind_141();
  kill_letbind_140();
  kill_letbind_139();
  kill_letbind_138();
  kill_letbind_137();
  kill_letbind_136();
  kill_letbind_135();
  kill_letbind_134();
  kill_letbind_133();
  kill_letbind_132();
  kill_letbind_131();
  kill_letbind_130();
  kill_letbind_129();
  kill_letbind_128();
  kill_letbind_127();
  kill_letbind_126();
  kill_letbind_125();
  kill_letbind_124();
  kill_letbind_123();
  kill_letbind_122();
  kill_letbind_121();
  kill_letbind_120();
  kill_letbind_119();
  kill_letbind_118();
  kill_letbind_117();
  kill_letbind_116();
  kill_letbind_115();
  kill_letbind_114();
  kill_letbind_113();
  kill_letbind_112();
  kill_letbind_111();
  kill_letbind_110();
  kill_letbind_109();
  kill_letbind_108();
  kill_letbind_107();
  kill_letbind_106();
  kill_letbind_105();
  kill_letbind_104();
  kill_letbind_103();
  kill_letbind_102();
  kill_letbind_101();
  kill_letbind_100();
  kill_letbind_99();
  kill_letbind_98();
  kill_letbind_97();
  kill_letbind_96();
  kill_letbind_95();
  kill_letbind_94();
  kill_letbind_93();
  kill_letbind_92();
  kill_letbind_91();
  kill_letbind_90();
  kill_letbind_89();
  kill_letbind_88();
  kill_letbind_87();
  kill_letbind_86();
  kill_letbind_85();
  kill_letbind_84();
  kill_letbind_83();
  kill_letbind_82();
  kill_letbind_81();
  kill_letbind_80();
  kill_letbind_79();
  kill_letbind_78();
  kill_letbind_77();
  kill_letbind_76();
  kill_letbind_75();
  kill_letbind_74();
  kill_letbind_73();
  kill_letbind_72();
  kill_letbind_71();
  kill_letbind_70();
  kill_letbind_69();
  kill_letbind_68();
  kill_letbind_67();
  kill_letbind_66();
  kill_letbind_65();
  kill_letbind_64();
  kill_letbind_63();
  kill_letbind_62();
  kill_letbind_61();
  kill_letbind_60();
  kill_letbind_59();
  kill_letbind_58();
  kill_letbind_57();
  kill_letbind_56();
  kill_letbind_55();
  kill_letbind_54();
  kill_letbind_53();
  kill_letbind_52();
  kill_letbind_51();
  kill_letbind_50();
  kill_letbind_49();
  kill_letbind_48();
  kill_letbind_47();
  kill_letbind_46();
  kill_letbind_45();
  kill_letbind_44();
  kill_letbind_43();
  kill_letbind_42();
  kill_letbind_41();
  kill_letbind_40();
  kill_letbind_39();
  kill_letbind_38();
  kill_letbind_37();
  kill_letbind_36();
  kill_letbind_35();
  kill_letbind_34();
  kill_letbind_33();
  kill_letbind_32();
  kill_letbind_31();
  kill_letbind_30();
  kill_letbind_29();
  kill_letbind_28();
  kill_letbind_27();
  kill_letbind_26();
  kill_letbind_25();
  kill_letbind_24();
  kill_letbind_23();
  kill_letbind_22();
  kill_letbind_21();
  kill_letbind_20();
  kill_letbind_19();
  kill_letbind_18();
  kill_letbind_17();
  kill_letbind_16();
  kill_letbind_15();
  kill_letbind_14();
  kill_letbind_13();
  kill_letbind_12();
  kill_letbind_11();
  kill_letbind_10();
  kill_letbind_9();
  kill_letbind_8();
  kill_letbind_7();
  kill_letbind_6();
  kill_letbind_5();
  kill_letbind_4();
  kill_letbind_3();
  kill_letbind_2();
  kill_letbind_1();
  kill_letbind_0();
  cleanup_rts();
  if (have_exception) {fprintf(stderr, "Exiting due to uncaught exception: %s\n", *throw_location);}
  KILL(zexception)(current_exception);
  sail_free(current_exception);
  KILL(sail_string)(throw_location);
  sail_free(throw_location);
  if (have_exception) {exit(EXIT_FAILURE);}
}



void model_test(void)
{
  for (size_t i = 0; i < SAIL_TEST_COUNT; ++i) {
    model_init();
    printf("Testing %s\n", SAIL_TEST_NAMES[i]);
    SAIL_TESTS[i](UNIT);
    printf("Pass\n");
    model_fini();
  }
}



#ifdef __cplusplus
}
#endif
