#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

const uint64_t GAS_COST_ZERO = UINT64_C(0);


struct GasCharge GAS_CHARGE_UNAFFORDABLE;


const uint8_t STATE_GAS_SPILL_ZERO = UINT8_C(0);


const uint8_t GAS_ZERO = UINT8_C(0);


const uint8_t STATE_GAS_ZERO = UINT8_C(0);


const uint32_t SYSTEM_CALL_GAS_LIMIT = UINT32_C(30000000);


const uint16_t GAS_CONSTANT_ZERO = UINT16_C(0);


const __int128 GAS_REFUND_ZERO = ((__int128)((((unsigned __int128)UINT64_C(0)) << 64) | UINT64_C(0)));


const __int128 FRAME_STATE_GAS_DELTA_ZERO = ((__int128)((((unsigned __int128)UINT64_C(0)) << 64) | UINT64_C(0)));


void create_letbind_11(void) {

  struct GasCharge let_value_3_16;
  struct GasCharge tmp_3_15;
  tmp_3_15.affordable = false;
  tmp_3_15.cost = GAS_COST_ZERO;
  let_value_3_16 = tmp_3_15;
  GAS_CHARGE_UNAFFORDABLE = let_value_3_16;
}
void kill_letbind_11(void) {
}

struct GasCharge gas_charge(uint64_t cost)
{
  return ((struct GasCharge){.affordable = true, .cost = cost});
}

