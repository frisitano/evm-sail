#pragma once

#include "evmsail/spec/primitives/quantities.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// struct GasCharge
struct GasCharge {
  bool affordable;
  uint64_t cost;
};

void create_letbind_11(void);
void kill_letbind_11(void);


struct GasCharge gas_charge(uint64_t cost);

extern const uint64_t GAS_COST_ZERO;


extern struct GasCharge GAS_CHARGE_UNAFFORDABLE;


extern const uint32_t STATE_GAS_SPILL_ZERO;


extern const uint64_t GAS_ZERO;


extern const uint32_t SYSTEM_CALL_GAS_LIMIT;


extern const uint16_t GAS_CONSTANT_ZERO;


extern const __int128 GAS_REFUND_ZERO;


extern const __int128 FRAME_STATE_GAS_DELTA_ZERO;



#ifdef __cplusplus
}
#endif
