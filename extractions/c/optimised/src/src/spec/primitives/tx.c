#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

struct BlobHashesFields EMPTY_BLOB_HASHES;


uint64_t tx_envelope_type(enum TxType t)
{
  switch (t) {
  case LegacyTx:
    return UINT64_C(0x00);
  case AccessListTx:
    return UINT64_C(0x01);
  case FeeMarketTx:
    return UINT64_C(0x02);
  case BlobTx:
    return UINT64_C(0x03);
  case SetCodeTx:
    return UINT64_C(0x04);
  }
}

struct TxTypeSemantics tx_type_semantics(enum TxType t)
{
  switch (t) {
  case LegacyTx:
    return ((struct TxTypeSemantics){.blob = false, .minimum_fork = Frontier, .set_code = false, .signature = LegacySignature});
  case AccessListTx:
    return ((struct TxTypeSemantics){.blob = false, .minimum_fork = Berlin, .set_code = false, .signature = TypedSignature});
  case FeeMarketTx:
    return ((struct TxTypeSemantics){.blob = false, .minimum_fork = London, .set_code = false, .signature = TypedSignature});
  case BlobTx:
    return ((struct TxTypeSemantics){.blob = true, .minimum_fork = Cancun, .set_code = false, .signature = TypedSignature});
  case SetCodeTx:
    return ((struct TxTypeSemantics){.blob = false, .minimum_fork = Prague, .set_code = true, .signature = TypedSignature});
  }
}

void create_letbind_73(void) {
    EMPTY_BLOB_HASHES = ((struct BlobHashesFields){.bytes = EMPTY_STATELESS_INPUT_SLICE, .count = UINT8_C(0)});
}
void kill_letbind_73(void) {
}

uint64_t log_store_index_increment(uint64_t value)
{
  if (value >= UINT64_C(18446744073709551615)) {
    __builtin_trap();
  }
  return (value + UINT64_C(1));
}

uint64_t log_store_index_add(uint64_t left, uint64_t right)
{
  if (right > (UINT64_C(18446744073709551615) - left)) {
    __builtin_trap();
  }
  return (right + left);
}

struct ReceiptFields receipt_fields(enum TxType tx_type, bool success, uint64_t gas_used, uint64_t execution_gas, uint64_t state_gas, struct LogSeriesRef logs)
{
  return ((struct ReceiptFields){.execution_gas = execution_gas, .gas_used = gas_used, .logs = logs, .state_gas = state_gas, .success = success, .tx_type = tx_type});
}

struct ReceiptFields receipt_within(enum TxType tx_type, bool success, uint64_t gas_used, uint64_t execution_gas, uint64_t state_gas, struct LogSeriesRef logs)
{
  return receipt_fields(tx_type, success, gas_used, execution_gas, state_gas, logs);
}

struct AuthorizationListRefFields authorization_list_ref(Bytes encoded, uint8_t count)
{
  return ((struct AuthorizationListRefFields){.count = (uint32_t)count, .encoded = encoded});
}

