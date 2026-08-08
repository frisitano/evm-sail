/* Optimized persistent-storage opcode implementations.
 *
 * State-passing convention: both operations receive the carried operand-stack
 * cursor and gas by value and return the carried gas; the Sail wrapper applies
 * the net cursor movement. Operands are read relative to the cursor (the row
 * one past the top), and every halt path returns the canonical carried-gas
 * exceptional halt. The frame registers are never touched here. */
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

uint64_t execute_sload(uint64_t top, uint64_t g)
{
  const u256 *rows = (const u256 *)(uintptr_t)top;
  const u256 slot = rows[-1];
  const bytes20 address = self_addr();
  const StorageId storage_id = storage_resolve_slot(slot);
  const bool warm = storage_id_is_warm(storage_id);
  const uint16_t gas_cost = sload_cost(warm);
  if (gas_cost > g) {
    return exc_halt(g, OutOfGas);
  }
  g -= gas_cost;

  storage_id_mark_warm(storage_id);
  const struct StorageValue entry =
      storage_load_by_id(k_parent_state_root, address, slot, storage_id);
  ((u256 *)(uintptr_t)top)[-1] = entry.curr;
  return g;
}

uint64_t execute_sstore(uint64_t top, uint64_t g)
{
  const struct ProtocolProfileFields profile = k_execution_profile.protocol;
  if (message.is_static) {
    return exc_halt(g, WriteProtection);
  }
  if (profile.fork < Amsterdam && g <= G_callstipend) {
    return exc_halt(g, OutOfGas);
  }

  const u256 *rows = (const u256 *)(uintptr_t)top;
  const u256 slot = rows[-1];
  const u256 value = rows[-2];
  const bytes20 address = self_addr();
  const StorageId storage_id = storage_resolve_slot(slot);
  const bool warm = storage_id_is_warm(storage_id);
  const bool cold = !warm;
  if (profile.fork >= Amsterdam) {
    const uint64_t sentry_cost = sstore_sentry_cost(cold);
    if (g < sentry_cost) {
      return exc_halt(g, OutOfGas);
    }
  }

  storage_id_mark_warm(storage_id);
  const struct StorageValue entry =
      storage_load_by_id(k_parent_state_root, address, slot, storage_id);
  const struct SstoreCosts costs = sstore_costs(entry.orig, entry.curr, value, cold);
  if (costs.state_credit != 0U) {
    g = credit_state_gas_refund(g, costs.state_credit);
  }
  if (costs.execution > g) {
    return exc_halt(g, OutOfGas);
  }
  g -= costs.execution;
  const struct tuple_bool_uint_64 debited =
      debit_state_gas_uint64_t_uint64_t_to_struct_tuple_bool_uint_64(g, costs.state_charge);
  g = debited.tup1;
  if (!debited.tup0) {
    return exc_halt(g, OutOfGas);
  }
  if (costs.refund != 0) {
    record_refund(costs.refund);
  }
  if (!word_equal(&entry.curr, &value)) {
    storage_update_by_id(storage_id, value, entry.orig);
  }
  return g;
}
