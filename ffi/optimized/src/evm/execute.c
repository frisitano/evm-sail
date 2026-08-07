/* Optimized persistent-storage opcode implementations. */
#include "evmsail/spec/evm/execute.h"
#include "evmsail/spec/evm/gas.h"
#include "evmsail/spec/evm/machine.h"
#include "evmsail/spec/kernel/environment.h"
#include "evmsail/spec/primitives/fork.h"

#include "host/state/internal.h"
#include "kernel/storage.h"
#include "primitives/value.h"

#include <stdbool.h>
#include <stdint.h>

static bool charge_execution_gas(uint64_t amount)
{
  if (amount <= gas_remaining) {
    gas_remaining -= amount;
    return true;
  }
  exc_halt(OutOfGas);
  return false;
}

void execute_sload(void)
{
  const u256 slot = pop();
  const bytes20 address = self_addr();
  const StorageId storage_id = storage_resolve_slot(slot);
  const bool warm = storage_id_is_warm(storage_id);
  const uint16_t gas_cost = sload_cost(warm);
  if (!charge_execution_gas(gas_cost)) {
    return;
  }

  storage_id_mark_warm(storage_id);
  const struct StorageValue entry =
      storage_load_by_id(k_parent_state_root, address, slot, storage_id);
  push_word(entry.curr);
}

void execute_sstore(void)
{
  const struct ProtocolProfileFields profile = k_execution_profile.protocol;
  if (guard_static()) {
    return;
  }
  if (profile.fork < Amsterdam && gas_remaining <= G_callstipend) {
    exc_halt(OutOfGas);
    return;
  }

  const u256 slot = pop();
  const u256 value = pop();
  const bytes20 address = self_addr();
  const StorageId storage_id = storage_resolve_slot(slot);
  const bool warm = storage_id_is_warm(storage_id);
  const bool cold = !warm;
  if (profile.fork >= Amsterdam) {
    const uint64_t sentry_cost = sstore_sentry_cost(cold);
    if (gas_remaining < sentry_cost) {
      exc_halt(OutOfGas);
      return;
    }
  }

  storage_id_mark_warm(storage_id);
  const struct StorageValue entry =
      storage_load_by_id(k_parent_state_root, address, slot, storage_id);
  const struct SstoreCosts costs = sstore_costs(entry.orig, entry.curr, value, cold);
  if (costs.state_credit != 0U) {
    credit_state_gas_refund(costs.state_credit);
  }
  if (!charge_execution_gas(costs.execution)) {
    return;
  }
  if (!debit_state_gas_uint64_t_to_bool(costs.state_charge)) {
    exc_halt(OutOfGas);
    return;
  }
  if (costs.refund != 0) {
    record_refund(costs.refund);
  }
  if (!word_equal(&entry.curr, &value)) {
    storage_update_by_id(storage_id, value, entry.orig);
  }
}
